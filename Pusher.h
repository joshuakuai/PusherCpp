/*
 The MIT License (MIT)
 
 Copyright (c) 2013 Jianghua Kuai
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PUSHER_H_
#define PUSHER_H_

#include <string>
#include <vector>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#define APNS_SANDBOX_HOST "gateway.sandbox.push.apple.com"
#define APNS_SANDBOX_PORT 2195

#define APNS_HOST "gateway.push.apple.com"
#define APNS_PORT 2195

#define DEVICE_BINARY_SIZE 32
#define MAXPAYLOAD_SIZE 256

using namespace std;

typedef struct _PusherContent{
	string content;
	int badge;
	string sound;
	string userData;
}PusherContent;

class Pusher {
public:
	Pusher(string cerFileName);
	virtual ~Pusher();
    
	bool isSandBox;
    
	void setExpirationDate(long expirationDate);
	void pushNotification(PusherContent pushContent,vector<string> tokenStringList);
    
private:
	string _cerFileName;
	vector<string> tokenStringVector;
    	long _expirationDate;
    
	string binaryToken(const std::string& input);
	int charToHex(char value);
	void prepareConnect(string pushContent);
	bool sendPayload(SSL *sslPtr, char *deviceTokenBinary, char *payloadBuff, size_t payloadLength);
};

#endif /* PUSHER_H_ */
