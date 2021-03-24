/*
 *  COPYRIGHT NOTICE  
 *  Copyright (C) 2015, Jhuster, All Rights Reserved
 *  Author: Jhuster(lujun.hust@gmail.com)
 *  
 *  https://github.com/Jhuster/TLV
 *   
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License, 
 *  or (at your option) any later version.
 */
#include <stdio.h>
#include <string.h>
#include "tlv_box.h"
uint16_t tlv_box_putobject(tlv_box_t *box, uint16_t type, void *value, uint16_t length);

uint16_t tlv_get_uint16data(uint8_t *data)
{
    union
    {
        uint8_t darr[2];
        uint16_t cdat;
    }tmpArr;
    tmpArr.darr[0] = data[0];
    tmpArr.darr[1] = data[1];
    return tmpArr.cdat;
}

static void tlv_box_release_tlv(value_t value)
{
    tlv_t *tlv = (tlv_t *)value.value;
    free(tlv->value);
    free(tlv);
}

tlv_box_t *tlv_box_create()
{
    tlv_box_t* box = (tlv_box_t*)malloc(sizeof(tlv_box_t));
    box->m_list = key_list_create(tlv_box_release_tlv);
    box->m_serialized_buffer = NULL;
    box->m_serialized_bytes = 0;
    return box;
}

tlv_box_t *tlv_box_parse(unsigned char *buffer, uint16_t buffersize)
{
    tlv_box_t *box = tlv_box_create();

    unsigned char *cached = (unsigned char*) malloc(buffersize);
    memcpy(cached, buffer, buffersize);

    uint16_t offset = 0, length = 0;
    while (offset < buffersize) {
        uint16_t type = tlv_get_uint16data(cached + offset);
        offset += sizeof(uint16_t);
        uint16_t length = tlv_get_uint16data(cached + offset);
        offset += sizeof(uint16_t);
        tlv_box_putobject(box, type, cached+offset, length);
        offset += length;
    }

    box->m_serialized_buffer = cached;
    box->m_serialized_bytes = buffersize;

    return box;
}

uint16_t tlv_box_destroy(tlv_box_t *box)
{
    key_list_destroy(box->m_list);
    if (box->m_serialized_buffer != NULL) {
        free(box->m_serialized_buffer);        
    }

    free(box);

    return 0;
}

unsigned char *tlv_box_get_buffer(tlv_box_t *box)
{
    return box->m_serialized_buffer;
}

uint16_t tlv_box_get_size(tlv_box_t *box)
{
    return box->m_serialized_bytes;
}

uint16_t tlv_box_putobject(tlv_box_t *box, uint16_t type, void *value, uint16_t length)
{
    if (box->m_serialized_buffer != NULL) {
        return -1;
    }

    tlv_t *tlv = (tlv_t *)malloc(sizeof(tlv_t));
    tlv->type = type;
    tlv->length = length;
    tlv->value = (unsigned char *) malloc(length);
    memcpy(tlv->value, value, length);

    value_t object;
    object.value = tlv;

    if (key_list_add(box->m_list, type, object) != 0) {
        return -1;
    }    
    box->m_serialized_bytes += sizeof(uint16_t) * 2 + length;
    
    return 0;
}

uint16_t tlv_box_put_char(tlv_box_t *box, uint16_t type, char value)
{
    return tlv_box_putobject(box, type, &value, sizeof(char));
}

uint16_t tlv_box_put_short(tlv_box_t *box, uint16_t type, short value)
{
    return tlv_box_putobject(box, type, &value, sizeof(short));
}

uint16_t tlv_box_put_int(tlv_box_t *box, uint16_t type, int value)
{
    return tlv_box_putobject(box, type, &value, sizeof(int));
}

uint16_t tlv_box_put_long(tlv_box_t *box, uint16_t type, long value)
{
    return tlv_box_putobject(box, type, &value, sizeof(long));
}

uint16_t tlv_box_put_longlong(tlv_box_t *box, uint16_t type, long long value)
{
    return tlv_box_putobject(box, type, &value, sizeof(long long));
}

uint16_t tlv_box_put_float(tlv_box_t *box, uint16_t type, float value)
{
    return tlv_box_putobject(box, type, &value, sizeof(float));
}

uint16_t tlv_box_put_double(tlv_box_t *box, uint16_t type, double value)
{
    return tlv_box_putobject(box, type, &value, sizeof(double));
}

uint16_t tlv_box_put_string(tlv_box_t *box, uint16_t type, char *value)
{
    return tlv_box_putobject(box, type, value, strlen(value)+1);
}

uint16_t tlv_box_put_bytes(tlv_box_t *box, uint16_t type, unsigned char *value, uint16_t length)
{
    return tlv_box_putobject(box, type, value, length);
}

uint16_t tlv_box_put_object(tlv_box_t *box, uint16_t type, tlv_box_t *object)
{
    return tlv_box_putobject(box, type, tlv_box_get_buffer(object), tlv_box_get_size(object));
}

uint16_t tlv_box_serialize(tlv_box_t *box)
{
    if (box->m_serialized_buffer != NULL) {
        return -1;
    }

    uint16_t offset = 0;
    unsigned char* buffer = (unsigned char*) malloc(box->m_serialized_bytes);    
    key_list_foreach(box->m_list, node) {
        tlv_t *tlv = (tlv_t *) node->value.value;        
        memcpy(buffer+offset, &tlv->type, sizeof(uint16_t));
        offset += sizeof(uint16_t);        
        memcpy(buffer+offset, &tlv->length, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        memcpy(buffer+offset, tlv->value, tlv->length);        
        offset += tlv->length;
    }

    box->m_serialized_buffer = buffer;

    return 0;
}

uint16_t tlv_box_get_char(tlv_box_t *box, uint16_t type, char *value)
{
    value_t object;
    if (key_list_get(box->m_list, type, &object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = (*(char *)(tlv->value));
    return 0;
}

uint16_t tlv_box_get_short(tlv_box_t *box, uint16_t type, short *value)
{
    value_t object;
    if (key_list_get(box->m_list, type, &object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = (*(short *)(tlv->value));
    return 0;
}

uint16_t tlv_box_get_int(tlv_box_t *box, uint16_t type, int *value)
{
    value_t object;
    if (key_list_get(box->m_list, type, &object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = (*(int *)(tlv->value));
    return 0;
}

uint16_t tlv_box_get_long(tlv_box_t *box, uint16_t type, long *value)
{
    value_t object;
    if (key_list_get(box->m_list, type, &object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = (*(long *)(tlv->value));
    return 0;
}

uint16_t tlv_box_get_longlong(tlv_box_t *box, uint16_t type, long long *value)
{
    value_t object;
    if (key_list_get(box->m_list, type, &object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = (*(long long *)(tlv->value));
    return 0;
}

uint16_t tlv_box_get_float(tlv_box_t *box, uint16_t type, float *value)
{
    value_t object;
    if (key_list_get(box->m_list, type, &object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = (*(float *)(tlv->value));
    return 0;
}

uint16_t tlv_box_get_double(tlv_box_t *box, uint16_t type, double *value)
{
    value_t object;
    if (key_list_get(box->m_list,type,&object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = (*(double *)(tlv->value));
    return 0;
}

uint16_t tlv_box_get_string(tlv_box_t *box, uint16_t type, char *value, uint16_t* length)
{
    return tlv_box_get_bytes(box,type,(unsigned char *)value,length);
}

uint16_t tlv_box_get_bytes(tlv_box_t *box, uint16_t type, unsigned char *value, uint16_t* length)
{
    value_t object;
    if (key_list_get(box->m_list,type,&object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    if (*length < tlv->length) {
        return -1;
    }
    memset(value, 0, *length);
    *length = tlv->length;
    memcpy(value, tlv->value, tlv->length);
    return 0;
}

uint16_t tlv_box_get_bytes_ptr(tlv_box_t *box, uint16_t type, unsigned char **value, uint16_t* length)
{
    value_t object;
    if (key_list_get(box->m_list, type, &object) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) object.value;
    *value = tlv->value;
    *length = tlv->length;
    return 0;
}

uint16_t tlv_box_get_object(tlv_box_t *box, uint16_t type, tlv_box_t **object)
{
    value_t value;
    if (key_list_get(box->m_list, type, &value) != 0) {
        return -1;
    }
    tlv_t *tlv = (tlv_t *) value.value;
    *object = (tlv_box_t *)tlv_box_parse(tlv->value, tlv->length);
    return 0;
}
