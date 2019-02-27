//
// Created by ad on 26.02.19.
//
#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#ifndef TCP_UTILS_H
#define TCP_UTILS_H



int invokeServer(char *buffer,int length,std::string text);
#endif //TCP_UTILS_H