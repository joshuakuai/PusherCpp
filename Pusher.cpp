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

#include "Pusher.h"
#include <algorithm>
#include <stdexcept>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <string.h>
#include <unistd.h>

Pusher::Pusher(string cerFileName) {
	_cerFileName = cerFileName;
	isSandBox = true;
    _expirationDate = time(NULL) + 86400; // default expiration date set to 1 day
}

Pusher::~Pusher() {
}

// Unix epoch date expressed in seconds (UTC) after which no more attempts should be made to deliver a notification. Set to 0 to expire immediately
void Pusher::setExpirationDate(long expirationDate) {
    _expirationDate = expirationDate;
	
}

int Pusher::charToHex(char value) {
	unsigned int x;
	stringstream ss;
	ss << std::hex << value;
	ss >> x;
	return x;
}

string Pusher::binaryToken(const std::string& input) {
	size_t len = input.length();
    
	//Convert the string to the hex vector string
	vector<char> inputCharVector;
	for (size_t i = 0; i < len; i++) {
		inputCharVector.push_back(this->charToHex(input[i]));
	}
    
	string output = "";
	char buffer[32];
	int location = 0;
	memset(buffer, 0, 32);
    
	unsigned value;
	unsigned data[4];
	for (size_t i = 0; i < len; i += 8) {
		memset(data, 0, 4);
		data[0] = (inputCharVector[i] << 4) | (inputCharVector[i + 1]);
		data[1] = (inputCharVector[i + 2] << 4) | (inputCharVector[i + 3]);
		data[2] = (inputCharVector[i + 4] << 4) | (inputCharVector[i + 5]);
		data[3] = (inputCharVector[i + 6] << 4) | (inputCharVector[i + 7]);
        
		value = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        
		value = htonl(value);
        
		memcpy(&buffer[location], &value, sizeof(unsigned));
        
		location += sizeof(unsigned);
	}
    
	return output.append(buffer, 32);
}

void Pusher::pushNotification(PusherContent pushContent,
                              vector<string> tokenStringList) {
    
	this->tokenStringVector = tokenStringList;
    
	if (this->tokenStringVector.size() > 0) {
		//convert the content to json string
		ostringstream stringStream;
		stringStream << "{\"aps\":{\"alert\":\"" << pushContent.content
        << "\",\"badge\":" << pushContent.badge << ",\"sound\":\""
        << pushContent.sound << "\"}";
		if (pushContent.userData.empty()) {
			stringStream << "}";
		} else {
			stringStream << "," << pushContent.userData << "}";
		}
        
		string contentString = stringStream.str();
        
		//cout << contentString << endl;
        
		this->prepareConnect(contentString);
	}
}

void Pusher::prepareConnect(string pushContent) {
	SSL_CTX *ctx;
	SSL *ssl;
	int sockfd;
	struct hostent *he;
	struct sockaddr_in sa;
    
	SSLeay_add_ssl_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(TLSv1_method());
	if (!ctx) {
		printf("SSL_CTX_new()...failed\n");
		exit(1);
	}
    
	if (SSL_CTX_load_verify_locations(ctx, NULL, ".") <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
    
	if (SSL_CTX_use_certificate_file(ctx, _cerFileName.c_str(),
                                     SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
    
	if (SSL_CTX_use_PrivateKey_file(ctx, _cerFileName.c_str(), SSL_FILETYPE_PEM)
        <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
    
	if (SSL_CTX_check_private_key(ctx) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket()...-1\n");
		exit(1);
	}
    
	sa.sin_family = AF_INET;
	if(this->isSandBox){
		he = gethostbyname(APNS_SANDBOX_HOST);
	}else{
		he = gethostbyname(APNS_HOST);
	}
    
	if (!he) {
		close(sockfd);
		exit(1);
	}
	sa.sin_addr.s_addr = inet_addr(
                                   inet_ntoa(*((struct in_addr *) he->h_addr_list[0])));
	if(this->isSandBox){
		sa.sin_port = htons(APNS_SANDBOX_PORT);
	}else{
		sa.sin_port = htons(APNS_PORT);
	}
    
	if (connect(sockfd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
		close(sockfd);
		exit(1);
	}
    
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sockfd);
	if (SSL_connect(ssl) == -1) {
		shutdown(sockfd, 2);
		close(sockfd);
		exit(1);
	}
    
	int successNumber = 0;
	int failedNumber = 0;
	unsigned int i = 0;
	for (; i < tokenStringVector.size(); i++) {
		if (!this->sendPayload(ssl,
                               (char*) this->binaryToken(tokenStringVector[i]).c_str(),
                               (char*) pushContent.c_str(), pushContent.length())) {
			failedNumber++;
		} else {
			successNumber++;
		}
	}
    
	cout << "Finished, total:" << i << ", success:" << successNumber
    << ",failed:" << failedNumber << "." << endl;
    
	SSL_shutdown(ssl);
	SSL_free(ssl);
	close(sockfd);
	SSL_CTX_free(ctx);
}

// Source: https://developer.apple.com/library/ios/documentation/NetworkingInternet/Conceptual/RemoteNotificationsPG/Chapters/LegacyFormat.html
bool Pusher::sendPayload(SSL *sslPtr, char *deviceTokenBinary, char *payloadBuff, size_t payloadLength) {
    bool rtn = false;
    if (sslPtr && deviceTokenBinary && payloadBuff && payloadLength) {
        uint8_t command = 1; /* command number */
        char binaryMessageBuff[sizeof (uint8_t) + sizeof (uint32_t) + sizeof (uint32_t) + sizeof (uint16_t) +
                               DEVICE_BINARY_SIZE + sizeof (uint16_t) + MAXPAYLOAD_SIZE];
        /* message format is, |COMMAND|ID|EXPIRY|TOKENLEN|TOKEN|PAYLOADLEN|PAYLOAD| */
        char *binaryMessagePt = binaryMessageBuff;
        uint32_t whicheverOrderIWantToGetBackInAErrorResponse_ID = 1234;
        uint32_t networkOrderExpiryEpochUTC = htonl(_expirationDate);
        uint16_t networkOrderTokenLength = htons(DEVICE_BINARY_SIZE);
        uint16_t networkOrderPayloadLength = htons(payloadLength);
        
        /* command */
        *binaryMessagePt++ = command;
        
        /* provider preference ordered ID */
        memcpy(binaryMessagePt, &whicheverOrderIWantToGetBackInAErrorResponse_ID, sizeof (uint32_t));
        binaryMessagePt += sizeof (uint32_t);
        
        /* expiry date network order */
        memcpy(binaryMessagePt, &networkOrderExpiryEpochUTC, sizeof (uint32_t));
        binaryMessagePt += sizeof (uint32_t);
        
        /* token length network order */
        memcpy(binaryMessagePt, &networkOrderTokenLength, sizeof (uint16_t));
        binaryMessagePt += sizeof (uint16_t);
        
        /* device token */
        memcpy(binaryMessagePt, deviceTokenBinary, DEVICE_BINARY_SIZE);
        binaryMessagePt += DEVICE_BINARY_SIZE;
        
        /* payload length network order */
        memcpy(binaryMessagePt, &networkOrderPayloadLength, sizeof (uint16_t));
        binaryMessagePt += sizeof (uint16_t);
        
        /* payload */
        memcpy(binaryMessagePt, payloadBuff, payloadLength);
        binaryMessagePt += payloadLength;
        
        int result = SSL_write(sslPtr, binaryMessageBuff,
                               (binaryMessagePt - binaryMessageBuff));
		if (result > 0) {
			rtn = true;
			//cout<< "SSL_write():" << result<< endl;
		} else {
			int errorCode = SSL_get_error(sslPtr, result);
			cout << "Failed to write in SSL, error code:" << errorCode << endl;
		}
    }
    return rtn;
}
