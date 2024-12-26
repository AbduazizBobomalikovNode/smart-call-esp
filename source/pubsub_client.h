#ifndef PUBSUB_CLIENT_H
#define PUBSUB_CLIENT_H

#include <PubSubClient.h>

typedef std::function<void()> callback_fun_t;

void callback(char* topic, byte* payload, unsigned int length);
void reconnect(callback_fun_t callback_fun);

#endif // PUBSUB_CLIENT_H
