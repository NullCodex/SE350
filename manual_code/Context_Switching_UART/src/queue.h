#ifndef QUEUE_H_
#define QUEUE_H_

#include "k_rtx.h"
#include "common.h"

int push(Queue* self, Element* element);
Element* pop(Queue* self);