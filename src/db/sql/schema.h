db_query(0, "create table bot_mask(   mid integer not null auto_increment,   uid integer not null,   mask varchar(255) not null,   primary key(mid))");
db_query(0, "create table bot_mode(   uid integer not null,   chan varchar(32) not null,   net varchar(32) not null,   action char(1) not null,   param varchar(255),   primary key(uid, chan, net, action))");
db_query(0, "create table bot_note(   nid integer not null auto_increment,   author integer not null,   target integer not null,   sent integer not null,   note varchar(255) not null,   primary key(nid))");
db_query(0, "create table bot_user(   uid integer not null auto_increment,   name varchar(64) not null,   pass varchar(64),   level integer,   primary key(uid))");
db_query(0, "create table bot_var(   uid integer not null,   name varchar(32) not null,   val varchar(255) not null,   primary key(uid, name))");
db_query(0, "create table bot_ban(   bid integer not null auto_increment,   net varchar(32) not null,   chan varchar(32) not null,   uid integer not null,   mask varchar(255) not null,   note varchar(80),   primary key(bid))");
