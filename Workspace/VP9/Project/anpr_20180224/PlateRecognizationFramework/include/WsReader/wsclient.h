#pragma once
#include "util.h"
#include "libwebsockets.h"

void setExitStt(bool stt);
struct lws_context *init_socket(string);
int deinit_socket(struct lws_context *);
int ws_ReadHandler(struct lws_context *);
bool isOpen();

void append_ws_msg(ws_message);
void downloadPacket(string);
download_info getNextPackage();
string getNextWsMessage();
