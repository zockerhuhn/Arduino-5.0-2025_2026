//
// OpenMV RPC (Remote Procedure Call) Library
// Copyright (c) 2020 OpenMV
// Written by Larry Bank & Kwabena W. Agyeman
//

#ifndef __OPENMVRPC__
#define __OPENMVRPC__

#include <Arduino.h>

namespace openmv {

typedef void (*rpc_callback_t)();
typedef void (*rpc_callback_with_args_t)(void *in_data, size_t in_data_len);
typedef void (*rpc_callback_returns_result_t)(void **out_data, size_t *out_data_len);
typedef size_t (*rpc_callback_returns_result_no_copy_t)(void *out_data);
typedef void (*rpc_callback_with_args_returns_result_t)(void *in_data, size_t in_data_len, void **out_data, size_t *out_data_len);
typedef size_t (*rpc_callback_with_args_returns_result_no_copy_t)(void *in_data, size_t in_data_len, void *out_data);
typedef bool (*rpc_stream_reader_callback_t)(uint8_t *in_data, uint32_t in_data_len);
typedef bool (*rpc_stream_writer_callback_t)(uint8_t **out_data, uint32_t *out_data_len);

extern uint8_t *__buff;
extern size_t __buff_len;

template <int size> class rpc_scratch_buffer
{
public:
    rpc_scratch_buffer() { __buff = buff; __buff_len = sizeof(buff); }
    ~rpc_scratch_buffer() {}
    size_t buffer_size() { return size; }
private:
    rpc_scratch_buffer(const rpc_scratch_buffer &);
    uint8_t buff[size + 4];
};

class rpc
{
public:
    rpc() {}
    ~rpc() {}
    virtual bool get_bytes(uint8_t *buff, size_t size, unsigned long timeout) = 0;
    virtual bool put_bytes(uint8_t *data, size_t size, unsigned long timeout) = 0;
    virtual void begin() {}
    virtual void end() {}
    bool stream_reader_setup(uint32_t queue_depth = 1);
    bool stream_reader_loop(rpc_stream_reader_callback_t callback, unsigned long read_timeout = 5000);
    bool stream_writer_setup();
    bool stream_writer_loop(rpc_stream_writer_callback_t callback, unsigned long write_timeout = 5000);
protected:
    const uint16_t _COMMAND_HEADER_PACKET_MAGIC = 0x1209;
    const uint16_t _COMMAND_DATA_PACKET_MAGIC = 0xABD1;
    const uint16_t _RESULT_HEADER_PACKET_MAGIC = 0x9021;
    const uint16_t _RESULT_DATA_PACKET_MAGIC = 0x1DBA;
    const unsigned long _put_long_timeout = 5000;
    const unsigned long _get_long_timeout = 5000;
    unsigned long _put_short_timeout;
    unsigned long _get_short_timeout;
    void _zero(uint8_t *data, size_t size);
    bool _same(uint8_t *data, size_t size);
    uint32_t _hash(const __FlashStringHelper *name);
    uint32_t _hash(const char *name, size_t length);
    uint32_t _hash(const char *name);
    bool _get_packet(uint16_t magic_value, uint8_t *buff, size_t size, unsigned long timeout);
    void _set_packet(uint8_t *buff, uint16_t magic_value, uint8_t *data, size_t size);
    virtual void _flush() {}
    virtual bool _stream_get_bytes(uint8_t *buff, size_t size, unsigned long timeout);
    virtual bool _stream_put_bytes(uint8_t *data, size_t size, unsigned long timeout);
    virtual uint32_t _stream_writer_queue_depth_max() { return 255; }
private:
    uint8_t __tx_lfsr;
    uint8_t __rx_lfsr;
    uint32_t __queue_depth;
    uint32_t __credits;
    rpc(const rpc &);
    uint16_t __crc_16(uint8_t *data, size_t size);
};

typedef enum rpc_callback_type {
    __CALLBACK,
    __CALLBACK_WITH_ARGS,
    __CALLBACK_RETURNS_RESULT,
    __CALLBACK_RETURNS_RESULT_NO_COPY,
    __CALLBACK_WITH_ARGS_RETURNS_RESULT,
    __CALLBACK_WITH_ARGS_RETURNS_RESULT_NO_COPY,
} rpc_callback_type_t;

typedef struct rpc_callback_entry {
    rpc *object;
    uint32_t key;
    rpc_callback_type_t type;
    void *value;
} rpc_callback_entry_t;

extern rpc_callback_entry_t *__dict;
extern size_t __dict_len;
extern size_t __dict_alloced;

template <int size> class rpc_callback_buffer
{
public:
    rpc_callback_buffer() { __dict = buff; __dict_len = size; __dict_alloced = 0; }
    ~rpc_callback_buffer() {}
    size_t buffer_size() { return size; }
    size_t buffer_free() { return size - __dict_alloced; }
    size_t buffer_used() { return __dict_alloced; }
private:
    rpc_callback_buffer(const rpc_callback_buffer &);
    rpc_callback_entry_t buff[size];
};

class rpc_master : public rpc
{
public:
    rpc_master() : rpc() {}
    ~rpc_master() {}
    bool call_no_copy_no_args(const __FlashStringHelper *name,
                              void **result_data=NULL, size_t *result_data_len=NULL,
                              unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_copy_no_args(const String &name,
                              void **result_data=NULL, size_t *result_data_len=NULL,
                              unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_copy_no_args(const char *name,
                              void **result_data=NULL, size_t *result_data_len=NULL,
                              unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_copy(const __FlashStringHelper *name,
                      void *command_data, size_t command_data_len,
                      void **result_data=NULL, size_t *result_data_len=NULL,
                      unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_copy(const String &name,
                      void *command_data, size_t command_data_len,
                      void **result_data=NULL, size_t *result_data_len=NULL,
                      unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_copy(const char *name,
                      void *command_data, size_t command_data_len,
                      void **result_data=NULL, size_t *result_data_len=NULL,
                      unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_args(const __FlashStringHelper *name,
                      void *result_data=NULL, size_t result_data_len=0, bool return_false_if_received_data_is_zero=true,
                      unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_args(const String &name,
                      void *result_data=NULL, size_t result_data_len=0, bool return_false_if_received_data_is_zero=true,
                      unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call_no_args(const char *name,
                      void *result_data=NULL, size_t result_data_len=0, bool return_false_if_received_data_is_zero=true,
                      unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call(const __FlashStringHelper *name,
              void *command_data, size_t command_data_len,
              void *result_data=NULL, size_t result_data_len=0, bool return_false_if_received_data_is_zero=true,
              unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call(const String &name,
              void *command_data, size_t command_data_len,
              void *result_data=NULL, size_t result_data_len=0, bool return_false_if_received_data_is_zero=true,
              unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
    bool call(const char *name,
              void *command_data, size_t command_data_len,
              void *result_data=NULL, size_t result_data_len=0, bool return_false_if_received_data_is_zero=true,
              unsigned long send_timeout=1000, unsigned long recv_timeout=1000);
protected:
    const unsigned long _put_short_timeout_reset = 3;
    const unsigned long _get_short_timeout_reset = 3;
private:
    rpc_master(const rpc_master &);
    uint8_t __in_command_header_buf[4];
    uint8_t __in_command_data_buf[4];
    uint8_t __out_result_header_ack[4];
    uint8_t __in_result_header_buf[8];
    uint8_t __out_result_data_ack[4];
    bool __put_command(uint32_t command, uint8_t *data, size_t size, unsigned long timeout);
    bool __get_result(uint8_t **data, size_t *size, unsigned long timeout);
};

class rpc_slave : public rpc
{
public:
    rpc_slave() : rpc() {}
    ~rpc_slave() {}
    bool register_callback(const __FlashStringHelper *name, rpc_callback_t callback);
    bool register_callback(const String &name, rpc_callback_t callback);
    bool register_callback(const char *name, rpc_callback_t callback);
    bool register_callback(const __FlashStringHelper *name, rpc_callback_with_args_t callback);
    bool register_callback(const String &name, rpc_callback_with_args_t callback);
    bool register_callback(const char *name, rpc_callback_with_args_t callback);
    bool register_callback(const __FlashStringHelper *name, rpc_callback_returns_result_t callback);
    bool register_callback(const String &name, rpc_callback_returns_result_t callback);
    bool register_callback(const char *name, rpc_callback_returns_result_t callback);
    bool register_callback(const __FlashStringHelper *name, rpc_callback_returns_result_no_copy_t callback);
    bool register_callback(const String &name, rpc_callback_returns_result_no_copy_t callback);
    bool register_callback(const char *name, rpc_callback_returns_result_no_copy_t callback);
    bool register_callback(const __FlashStringHelper *name, rpc_callback_with_args_returns_result_t callback);
    bool register_callback(const String &name, rpc_callback_with_args_returns_result_t callback);
    bool register_callback(const char *name, rpc_callback_with_args_returns_result_t callback);
    bool register_callback(const __FlashStringHelper *name, rpc_callback_with_args_returns_result_no_copy_t callback);
    bool register_callback(const String &name, rpc_callback_with_args_returns_result_no_copy_t callback);
    bool register_callback(const char *name, rpc_callback_with_args_returns_result_no_copy_t callback);
    bool loop(unsigned long recv_timeout=100, unsigned long send_timeout=100);
protected:
    const unsigned long _put_short_timeout_reset = 2;
    const unsigned long _get_short_timeout_reset = 2;
private:
    rpc_slave(const rpc_slave &);
    uint8_t __in_command_header_buf[12];
    uint8_t __out_command_header_ack[4];
    uint8_t __out_command_data_ack[4];
    uint8_t __in_response_header_buf[4];
    uint8_t __in_response_data_buf[4];
    bool __get_command(uint32_t *command, uint8_t **data, size_t *size, unsigned long timeout);
    bool __put_result(uint8_t *data, size_t size, unsigned long timeout);
    bool __register_callback(uint32_t hash, rpc_callback_type_t type, void *value);
};

class rpc_hardware_serial1_uart_master : public rpc_master
{
public:
    rpc_hardware_serial1_uart_master(unsigned long baudrate=115200) : rpc_master(), __baudrate(baudrate) {}
    ~rpc_hardware_serial1_uart_master() {}
    virtual void _flush() override;
    virtual bool get_bytes(uint8_t *buff, size_t size, unsigned long timeout) override;
    virtual bool put_bytes(uint8_t *data, size_t size, unsigned long timeout) override;
    virtual void begin() override { Serial1.begin(__baudrate); }
    virtual void end() override { Serial1.end(); }
private:
    unsigned long __baudrate;
    rpc_hardware_serial1_uart_master(const rpc_hardware_serial1_uart_master &);
};

class rpc_hardware_serial1_uart_slave : public rpc_slave
{
public:
    rpc_hardware_serial1_uart_slave(unsigned long baudrate=115200) : rpc_slave(), __baudrate(baudrate) {}
    ~rpc_hardware_serial1_uart_slave() {}
    virtual void _flush() override;
    virtual bool get_bytes(uint8_t *buff, size_t size, unsigned long timeout) override;
    virtual bool put_bytes(uint8_t *data, size_t size, unsigned long timeout) override;
    virtual void begin() override { Serial1.begin(__baudrate); }
    virtual void end() override { Serial1.end(); }
private:
    unsigned long __baudrate;
    rpc_hardware_serial1_uart_slave(const rpc_hardware_serial1_uart_slave &);
};

} // namespace openmv

#endif // __OPENMVRPC__
