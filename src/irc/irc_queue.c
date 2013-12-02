/*
 * src/irc/irc_queue.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Handles IRC output queue.
 *
 * Flood control is done according to RFC2813 or RFC1459.
 *   Currently, it does not distinguish between commands, so no
 *   penalty modifiers are used for certain commands.
 *
 * Implemented as a binary heap priority queue.
 *
 *   Priorities need to be positive.
 *   Priority 0 will be dropped.
 *   Priority 9 or more will be sent immediately.
 *   Priority 2-8 will be queued until they can be sent without flooding,
 *      and items with the same priority will be sent in the order
 *      received.
 *
 * If the heap is full, it will be expanded (doubled in size).
 *
 * First entry of array is empty, to make calculations easier at the
 *    expense of a wasted 512 byte + sizeof(int) entry.
 *
 * Hopefully memcpy() is fairly efficient at moving 512 bytes around.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "irc_network.h"
#include "irc_queue.h"

struct item
{
   unsigned int p;
   char d[512];
};

static int enqueue(int priority, char *text);
static int dequeue(char *buf);

static int heap_capacity, heap_size;
static unsigned int heap_position;
static struct item *heap_item;

static int timer;

int _irc_queue_init(int capacity)
{
   if(capacity == -1)
   {
      heap_capacity = -1;
      heap_size = 1;
      return 0;
   }

   if(capacity < 1)
      capacity = 32;

   /* first entry will be empty, so make this larger */
   capacity++;

   heap_item = malloc(sizeof(struct item) * capacity);

   if(heap_item == NULL)
      return -1;

   heap_capacity = capacity;
   heap_size = 1;
   heap_position = 0x1fffffff;

   timer = 0;

   return 0;
}

int _irc_queue_update(void)
{
   char buf[512];
   int now;

   /* empty queue */
   if(heap_size == 1)
      return 0;

   /* see section 5.8 of rfc2813 for description */

   now = time(0);

   /* update timer if needed */
   if(timer < now)
      timer = now;

   /* if timer is more than 10 seconds away, wait. */
   if(timer >= (now + 10))
      return 0;

   /* penalize 2 seconds */
   timer += 2;

   if(dequeue(buf) != 0)
      return -1;

   _irc_net_send(buf);

   return 0;
}

int _irc_queue_insert(int priority, char *format, ...)
{
   char buffer[512];
   va_list ap;
   int now;

   va_start(ap, format);
   vsnprintf(buffer, 510, format, ap);
   va_end(ap);

   /* special case: no queueing */
   if(heap_capacity == -1)
   {
      _irc_net_send(buffer);
      return 0;
   }

   /* throw out anything with priority 0 or less */
   if(priority < 1)
      return 0;

   now = time(0);

   /* send anything with priority 9 or more right now */
   if(priority > 8)
   {
      if(timer < now)
         timer = now;

      timer += 2;
      _irc_net_send(buffer);
      return 0;
   }

   /* otherwise, enqueue */
   return enqueue(priority, buffer);
}

void _irc_queue_purge(void)
{
   heap_size = 1;
   heap_position = 0x1fffffff;
}

static int enqueue(int priority, char *text)
{
   unsigned int i, p;

   /* grow array if needed */
   if(heap_size >= heap_capacity)
   {
      struct item *p;

      heap_capacity *= 2;
      heap_capacity--;

      p = realloc(heap_item, sizeof(struct item) * heap_capacity);

      if(p == NULL)
         return -1;

      heap_item = p;
   }

   /* change from 1 -> 8 to 0 -> 7 */
   p = priority - 1;

   /* move priority level to left 3 bits */
   p <<= 29;

   /* put counter into right 29 bits */
   p |= heap_position;

   /* update counter */
   heap_position--;

   i = heap_size;

   /* ensure that added item won't destroy heap */
   while((i > 1) && (heap_item[i/2].p < p))
   {
      heap_item[i].p = heap_item[i/2].p;
      memcpy(heap_item[i].d, heap_item[i/2].d, 512);

      i /= 2;
   }

   /* insert into heap */
   heap_item[i].p = p;
   strncpy(heap_item[i].d, text, 512);
   heap_item[i].d[511] = 0;

   heap_size++;

   return 0;
}

static int dequeue(char *buf)
{
   unsigned int parent, child;
   struct item node;

   if(heap_size == 1)
      return -1;

   /* root is next */
   if(buf != NULL)
      memcpy(buf, heap_item[1].d, 512);

   heap_size--;

   /* displaced node */
   node.p = heap_item[heap_size].p;
   memcpy(node.d, heap_item[heap_size].d, 512);

   parent = 1;

   while(parent <= (heap_size / 2))
   {
      child = 2 * parent;

      /* promote child with greater priority */
      if((child < heap_size) &&
         (heap_item[child].p < heap_item[child + 1].p))
         child++;

      /* if displaced node is more important, stick it here */
      if(heap_item[child].p <= node.p)
         break;

      /* move child upwards */
      heap_item[parent].p = heap_item[child].p;
      memcpy(heap_item[parent].d, heap_item[child].d, 512);

      /* continue downwards */
      parent = child;
   }

   /* put displaced node back into tree */
   heap_item[parent].p = node.p;
   memcpy(heap_item[parent].d, node.d, 512);

   /* reset position if heap is empty */
   if(heap_size == 1)
      heap_position = 0x1fffffff;

   return 0;
}
