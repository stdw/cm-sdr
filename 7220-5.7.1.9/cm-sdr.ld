memset = 0x80522d7c;
memcpy = 0x80004f30;
malloc = 0x80596998;
printf = 0x8052b178;
socket = 0x80332fd0;
close = 0x80412978;
bind = 0x800ae7bc;
listen = 0x80412ed4;
accept = 0x80413118;
send = 0x80413240;
recv = 0x804134bc;
recvfrom = 0x804135cc;
sendto = 0x804133d8;
create_thread = 0x80387ecc;
start_thread = 0x80388198;
tune_aux_channel = 0x80082108;
int2double = 0x80010f30;
double_div = 0x80010c98;
sleep = 0x802f3a88;
sleep2 = 0x8045d808;
sem_init = 0x802f3a10;
sem_wait = 0x80409384;
sem_post = 0x80409500;
sem_getcount = 0x804095e8;

SECTIONS
{
  . = 0x80810000;
  .start : { *(.start) }
  .text : { *(.text) }
  .data : { *(.data) }
  .rodata : { *(.rodata) }
}
