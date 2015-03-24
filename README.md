PusherCpp
======

A C++ class to push notification to iOS Device

This class is simple to use, just get a instance of it, set the pem file name and the content, and push!

And it helps you to format your content.

Example:
```cpp
Pusher pusher("xxxxxx.pem");
pusher.isSandBox = true;

vector<string> tokenStringList;
tokenStringList.push_back("70bd309357cb5ee5c15e63a386d1defe5e1006a10aa949caf833e768c6472deb");
tokenStringList.push_back("d411103cfac7027894c91c94a03d9e00f081659f689e7af913c5f48d807b8546");

PusherContent content;
content.badge = 1;
content.content = "Test_for_pusher!";
content.sound = "default";
content.userData = "\"UserData\":123";

pusher.pushNotification(content,tokenStringList);
```
Before you start to use this class, make sure you've followed these steps and get your pem file ready.

* You need a certificate from Apple, and then double click it to install on your mac.

* Open the Keychain, then find the certificate you just installed.

* Right click that certificate and export it as a p12 file, name it as apns-cert.p12 or whatever you want.

* Export that certificate’s private key as a p12 file, name it as apns-key.p12 or whatever you want.

* Then transfer the both file to .pem file with command below.

   **openssl pkcs12 -clcerts -nokeys -out apns-cert.pem -in apns-cert.p12**

   **openssl pkcs12 -nocerts -out apns-key.pem -in apns-key.p12**

* Remove the password of the key file.

   **openssl rsa -in apns-key.pem -out apns-key-noenc.pem**

* Finally, clue them together.

   **cat apns-cert.pem apns-key-noenc.pem > apns-dev.pem**

* This class only support the token without space, you could use code below to get the token in iOS client.
```
NSString *hexString = [[[[deviceToken description] stringByReplacingOccurrencesOfString:@”<” withString:@””] stringByReplacingOccurrencesOfString:@”>” withString:@””]stringByReplacingOccurrencesOfString:@” ” withString:@””];
```

