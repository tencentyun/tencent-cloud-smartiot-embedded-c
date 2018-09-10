#include "tc_iot_inc.h"

static bool tc_iot_http_has_line_ended(const char * str) {
    while (*str) {
        if (*str == '\r') {
            return true;
        }
        str ++;
    }
    return false;
}

void tc_iot_http_parser_init(tc_iot_http_response_parser * parser) {
    if (parser) {
        parser->state = _PARSER_START;
        parser->version = 0;
        parser->status_code = 0;
        parser->content_length = 0;
        parser->location = NULL;
        /* parser->body = NULL; */
    }
}

int tc_iot_http_parser_analysis(tc_iot_http_response_parser * parser, const char * buffer, int buffer_len) {
    bool header_complete = false;
    const char * pos = NULL;
    int buffer_parsed = 0;
    int i = 0;

    IF_NULL_RETURN(parser, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);

    pos = buffer;
start:
    if (buffer_parsed >= buffer_len) {
        return buffer_parsed;
    }

    switch(parser->state) {
        case _PARSER_START:
            /* Head should alwarys start with: HTTP/1.x COD */
		    if ( buffer_len < sizeof("HTTP/1.x M0N")) {
                return 0;
            }

            if (!tc_iot_str7equal(pos, 'H', 'T', 'T', 'P', '/', '1', '.')) {
                TC_IOT_LOG_ERROR("Response header not start with HTTP/1.[x] .");
                return TC_IOT_FAILURE;
            } else {
                buffer_parsed += 7;
                pos = buffer + buffer_parsed;
            }

            if (pos[0] != '0' && pos[0] != '1') {
                TC_IOT_LOG_ERROR("HTTP version 1.%c not supported", pos[0]);
                return TC_IOT_FAILURE;
            } else {
                parser->version = pos[0] - '0';
            }

            if (' ' != pos[1]) {
                TC_IOT_LOG_ERROR("space not found");
                return TC_IOT_FAILURE;
            }

            buffer_parsed += 2;
            pos = buffer + buffer_parsed;
            parser->status_code = 0;
            for (i = 0; i < 3; i++) {
                if (pos[i] > '9' || pos[i] < '0') {
                    return TC_IOT_HTTP_INVALID_STATUS_CODE;
                } else {
                    parser->status_code = parser->status_code * 10 + pos[i] - '0';
                }
            }
            TC_IOT_LOG_TRACE("version: 1.%d, status code: %d",parser->version, parser->status_code);
            buffer_parsed += 3;
            pos = buffer + buffer_parsed;
            parser->state = _PARSER_IGNORE_TO_RETURN_CHAR;
            goto start;
        case _PARSER_IGNORE_TO_RETURN_CHAR:
            while (buffer_parsed < buffer_len) {
                if ('\r' == (*pos)) {
                    buffer_parsed++;
                    pos++;
                    parser->state = _PARSER_SKIP_NEWLINE_CHAR;
                    goto start;
                    break;
                }
                buffer_parsed++;
                pos++;
            }
            return buffer_parsed;
        case _PARSER_SKIP_NEWLINE_CHAR:
            if ('\n' != *pos) {
                TC_IOT_LOG_ERROR("expecting \\n");
                return TC_IOT_FAILURE;
            }
            buffer_parsed++;
            pos++;
            parser->state = _PARSER_HEADER;
            goto start;
        case _PARSER_HEADER:
            /* TC_IOT_LOG_TRACE("pos=%s",pos); */
            if ('\r' == (*pos)) {
                /* status line\r\n */
                /* http headers\r\n */
                /* \r\nbody */
                /* ^ */
                /* | we are here */
                buffer_parsed += 1;
                pos = buffer + buffer_parsed;
                parser->state = _PARSER_IGNORE_TO_BODY_START;
                goto start;
            } else {
                for (i = 0; i < (buffer_len-buffer_parsed); i++) {
                    if (':' == pos[i]) {
                        if ((i == tc_iot_const_str_len(HTTP_HEADER_CONTENT_LENGTH))
                                && (0 == memcmp(pos, HTTP_HEADER_CONTENT_LENGTH, i))) {
                            header_complete = tc_iot_http_has_line_ended(pos+i+1);
                            if (header_complete) {
                                /* TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_CONTENT_LENGTH, pos+i+2); */
                                parser->content_length = tc_iot_try_parse_int(pos+i+2, NULL);
                            } else {
                                TC_IOT_LOG_TRACE("%s not complete, continue reading:%s",HTTP_HEADER_CONTENT_LENGTH, pos+i+2);
                                return buffer_parsed;
                            }

                        } else if ((i == tc_iot_const_str_len(HTTP_HEADER_LOCATION))
                                && (0 == memcmp(pos, HTTP_HEADER_LOCATION, i))) {
                            header_complete = tc_iot_http_has_line_ended(pos+i+1);
                            if (header_complete) {
                                TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_LOCATION, pos+i+2);
                                parser->location = pos+i+2;
                            } else {
                                TC_IOT_LOG_TRACE("%s not complete, continue reading:%s",HTTP_HEADER_LOCATION, pos+i+2);
                                return buffer_parsed;
                            }
                        } else if ((i == tc_iot_const_str_len(HTTP_HEADER_CONTENT_TYPE))
                                && (0 == memcmp(pos, HTTP_HEADER_CONTENT_TYPE, i))) {
                            /* TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_CONTENT_TYPE, pos+i+2); */
                        } else {
                            /* TC_IOT_LOG_TRACE("ignore i=%d,pos=%s",i, pos); */
                        }
                        buffer_parsed += i+1;
                        pos = buffer + buffer_parsed;
                        parser->state = _PARSER_IGNORE_TO_RETURN_CHAR;
                        goto start;
                    }
                }
                TC_IOT_LOG_TRACE("buffer_parsed=%d, buffer_len=%d,left=%s", buffer_parsed, buffer_len, pos);
                return buffer_parsed;
            }
            break;
        case _PARSER_IGNORE_TO_BODY_START:
            if ('\n' != *pos) {
                TC_IOT_LOG_ERROR("expecting \\n");
                return TC_IOT_FAILURE;
            }
            buffer_parsed++;
            pos++;
            /* TC_IOT_LOG_TRACE("body=%s", pos); */
            parser->state = _PARSER_END;
            /* parser->body = pos; */
            return buffer_parsed;
        case _PARSER_END:
            return buffer_len;
        default:
            TC_IOT_LOG_ERROR("invalid parse state=%d", parser->state);
            return TC_IOT_FAILURE;
    }
}


int tc_iot_http_client_init(tc_iot_http_client * c, const char * method) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);

    memset(c, 0, sizeof(*c));
    if (!method) {
        c->method = HTTP_POST;
    } else {
        c->method = method;
    }
    c->version = HTTP_VERSION_1_1;
    c->abs_path = "/";
    c->extra_headers = "";
    c->content_type = HTTP_CONTENT_FORM_URLENCODED;

    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_version(tc_iot_http_client * c, const char * version) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->version = version;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_host(tc_iot_http_client * c, const char * host) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->host = host;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_abs_path(tc_iot_http_client * c, const char * abs_path) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->abs_path = abs_path;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_content_type(tc_iot_http_client * c, const char * content_type) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->content_type = content_type;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_extra_headers(tc_iot_http_client * c, const char * extra_headers) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->extra_headers = extra_headers;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_body(tc_iot_http_client * c, const char * body) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->body = body;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_format_buffer(char * buffer, int buffer_len, tc_iot_http_client * c) {
    int ret = 0;
    int content_length = 0;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_LESS_RETURN(buffer_len, 0, TC_IOT_INVALID_PARAMETER);

    if (c->body) {
        content_length = strlen(c->body);
    } else {
        c->body = "";
    }

    ret = tc_iot_hal_snprintf(buffer, buffer_len,
                              "%s %s HTTP/%s\r\n"
                              "User-Agent: tciotclient/1.0\r\n"
                              "Host: %s\r\n"
                              "Accept: */*\r\n"
                              "Content-Type: %s\r\n"
                              "Accept-Encoding: identity\r\n"
                              "Content-Length: %d\r\n%s\r\n%s",
                              c->method, c->abs_path, c->version,
                              c->host,c->content_type,content_length,
                              c->extra_headers,c->body);
    if (ret > 0) {
        return ret;
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
}



int tc_iot_http_client_internal_perform(char * buffer, int buffer_used, int buffer_len,
                                        tc_iot_network_t * p_network, tc_iot_http_response_parser * p_parser,
                                        const char * host, uint16_t port,
                                        bool secured, int timeout_ms,
                                        tc_iot_http_response_callback resp_callback, const void * callback_context) {
    bool head_only = false;
    const int timer_tick = 100;
    int ret = 0;
    int write_ret = 0;
    int read_ret  = 0;
    int parse_ret = 0;
    int callback_ret = 0;
    tc_iot_timer timer;
    int parse_left = 0;
    int content_length = 0;
    int received_bytes = 0;

    ret = tc_iot_network_prepare(p_network, TC_IOT_SOCK_STREAM, TC_IOT_PROTO_HTTP, secured);
    if (ret < 0) {
        return ret;
    }

    head_only = tc_iot_str4equal(buffer, 'H', 'E', 'A', 'D');

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer,timeout_ms);
    ret = p_network->do_connect(p_network, host, port);
    write_ret = p_network->do_write(p_network, (unsigned char *)buffer, buffer_used, timeout_ms);
    if (write_ret != buffer_used) {
        TC_IOT_LOG_ERROR("send packet failed: expect len=%d, write return=%d", buffer_used, write_ret)
        return TC_IOT_SEND_PACK_FAILED;
    }

    tc_iot_http_parser_init(p_parser);
    do {
        read_ret = p_network->do_read(p_network, (unsigned char *)buffer+parse_left, buffer_len-parse_left, timer_tick);
        if (read_ret > 0) {
            TC_IOT_LOG_TRACE("read_ret=%d,parse_left=%d", read_ret, parse_left);

            read_ret += parse_left;
            if (read_ret < buffer_len) {
                buffer[read_ret] = '0';
            }

            parse_ret = tc_iot_http_parser_analysis(p_parser, buffer, read_ret);
            if (parse_ret < 0) {
                TC_IOT_LOG_ERROR("read from request host=%s:%d failed, ret=%d", host, port, ret);
                p_network->do_disconnect(p_network);
                return parse_ret;
            }

            if (parse_ret > read_ret) {
                TC_IOT_LOG_ERROR("tc_iot_http_parser_analysis parse_ret=%d too large, ret=%d", parse_ret, ret);
                p_network->do_disconnect(p_network);
                return TC_IOT_FAILURE;
            }

            parse_left = read_ret - parse_ret;
            if (parse_left > 0) {
                memmove(buffer, buffer+parse_ret, parse_left);
                buffer[parse_left] = '\0';
            }

            if (p_parser->status_code != 200 && p_parser->status_code != 206) {
                TC_IOT_LOG_ERROR("http resoponse parser.status_code = %d", p_parser->status_code);
                p_network->do_disconnect(p_network);
                return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
            }

            if (_PARSER_END == p_parser->state) {
                if (head_only) {
                    TC_IOT_LOG_TRACE("this is a head request, quit body parsing.");
                    return TC_IOT_SUCCESS;
                }
                content_length = p_parser->content_length;
                received_bytes = parse_left;
                TC_IOT_LOG_TRACE("ver=1.%d, code=%d,content_length=%d, received_bytes=%d",
                                 p_parser->version, p_parser->status_code, p_parser->content_length, received_bytes);
                if (content_length > buffer_len) {
                    if (resp_callback == NULL) {
                        TC_IOT_LOG_ERROR("buffer not enough: content_length=%d, buffer_len=%d", content_length, buffer_len);
                        p_network->do_disconnect(p_network);
                        return TC_IOT_BUFFER_OVERFLOW;
                    } else {

                        callback_ret = resp_callback(callback_context, (const char *)buffer, parse_left, received_bytes , content_length);
                        if (callback_ret != TC_IOT_SUCCESS) {
                            TC_IOT_LOG_ERROR("callback failed ret=%d, abort.", callback_ret);
                            return TC_IOT_FAILURE;
                        }

                        while (received_bytes < content_length) {
                            // buffer_len-1 是为了预留一个字节，用来添加字符串结束符 '\0' 。
                            read_ret = p_network->do_read(p_network, (unsigned char *)buffer, buffer_len-1, timer_tick);
                            if (read_ret > 0) {
                                received_bytes += read_ret;
                                TC_IOT_LOG_TRACE("read=%d,total_read=%d, total=%d", read_ret, received_bytes, content_length);

                                buffer[read_ret] = '\0';
                                callback_ret = resp_callback(callback_context, (const char *)buffer, read_ret, received_bytes , content_length);
                                if (callback_ret != TC_IOT_SUCCESS) {
                                    TC_IOT_LOG_ERROR("callback failed ret=%d, abort.", callback_ret);
                                    return TC_IOT_FAILURE;
                                }
                                received_bytes += ret;

                                if (received_bytes >= content_length) {
                                    TC_IOT_LOG_TRACE("%s=%d, received_bytes=%d", HTTP_HEADER_CONTENT_LENGTH, content_length, received_bytes);
                                    p_network->do_disconnect(p_network);
                                    return TC_IOT_SUCCESS;
                                }
                            } else if (read_ret == TC_IOT_NET_NOTHING_READ) {
                                continue;
                            } else {
                                TC_IOT_LOG_ERROR("read buffer error:%d", read_ret);
                                p_network->do_disconnect(p_network);
                                return read_ret;
                            }
                        }
                        p_network->do_disconnect(p_network);
                        return received_bytes;
                    }
                }

                while (received_bytes < content_length) {
                    read_ret = p_network->do_read(p_network, (unsigned char *)buffer+parse_left, buffer_len-parse_left, timer_tick);
                    if (read_ret > 0) {
                        received_bytes += read_ret;
                        TC_IOT_LOG_TRACE("read=%d,total_read=%d, total=%d", read_ret, received_bytes, content_length);
                    } else if (read_ret == TC_IOT_NET_NOTHING_READ) {
                        continue;
                    } else {
                        TC_IOT_LOG_ERROR("read buffer error:%d", read_ret);
                        p_network->do_disconnect(p_network);
                        return read_ret;
                    }
                }
                p_network->do_disconnect(p_network);
                return received_bytes;
            }
        } else if (read_ret == TC_IOT_NET_NOTHING_READ) {
            continue;
        } else {
            TC_IOT_LOG_ERROR("read buffer error:%d", read_ret);
            p_network->do_disconnect(p_network);
            return read_ret;
        }

        if (read_ret <= 0) {
            TC_IOT_LOG_TRACE("ret=%d, len=%d", read_ret, buffer_len);
            p_network->do_disconnect(p_network);
            return TC_IOT_SUCCESS;
        }

    } while(!tc_iot_hal_timer_is_expired(&timer) && (received_bytes < buffer_len));

    p_network->do_disconnect(p_network);

    if (received_bytes > 0) {
        return received_bytes;
    } else {
        return read_ret;
    }
}

int tc_iot_http_client_perform(char * buffer, int buffer_used, int buffer_len,
                            const char * host, uint16_t port,
                            bool secured, int timeout_ms) {
    tc_iot_network_t network;
    tc_iot_http_response_parser parser;
    return tc_iot_http_client_internal_perform(buffer, buffer_used, buffer_len,
                                               &network, &parser,  host, port,
                                               secured, timeout_ms, NULL, NULL);
}
