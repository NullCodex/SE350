#include "queue.h"
#include "rtx.h"

Element* pop(Queue* self) {
    Element* element = NULL;

    if (self == NULL || self->first == NULL) {
        return NULL;
    } else if (self->first->next == NULL) {
        element = self->first;
        element->next = NULL;
        self->first = NULL;
        self->last = NULL;
        return element;
    }
    element = self->first;
    self->first = self->first->next;
    element->next = NULL;
    return element;
};

int push(Queue* self, Element* element) {
    element->next = NULL;

    if (self->first == NULL) {
            self->first = element;
            self->last = element;
    } else {
        self->last->next = element;
        self->last = element;
    }
    self->last->next = NULL;

    return RTX_OK;
};