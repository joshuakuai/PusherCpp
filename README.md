PusherCpp
======

A C++ class to push notification to iOS Device

This class is simple to use, just get a instance of it, set the pem file name and the content, and push!

And it helps you to format your content.

Wait....if you wanna make sure your pem file is right and could work correctly with this class, please take a few mins to take a look at my blog.

Blog address:http://www.joshuakuai.com/?p=1342

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

