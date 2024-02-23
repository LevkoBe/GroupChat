## Implementation of group chat

This is my implementation of an app that serves multiple clients simultanuously, with possibility to connect to certain 'rooms'.
And before going into details about general functionality, I'd like to describe the transmission protocol of the program.

### Transmission protocol documentation

My realization of connection between client and server uses TCP protocol, adding some extra-features.
First of all, each data transmission should follow the next guideline:

Each time we should send, as well as receive the next:

| Type of operation | CHAR | 1 B |
| --- | --- | --- |
| Size of chunk | INT | 4 B |
| Size of message | INT | 4 B |
| Message | CHAR* | x B |

**Type of operation** is the first message that is being sent, and it represent some predefined command, that should be recognizable by the receiver. Alternatively, it can be ignored, if all the receiver needs is to receive some data.

**Size of chunk** is an integer, representing the size of chunks in which the message sent is being divided. May be used by receiver to get data in the same-sized chunks as ones being sent.

**Size of message** is also an integer and defines the size of the whole message that needs to be received. Vital for avoiding errors in receiving partial data, receiving data from another message, or expecting to receive data when it’s no longer being send.

**Message** is any amount of characters stored in char pointer type. That is the message with specified size, which is expected by the receiver.
Here's a simple visualizations related to my program:

![image](https://github.com/LevkoBe/GroupChat/assets/118983753/f31d3f71-ad1f-494d-9f39-6c5a3be9beef) ![image](https://github.com/LevkoBe/GroupChat/assets/118983753/dadb8fb7-ab25-4326-a27f-ecb917373b4a)


Here's a markdown table representing the commands exchanged between the server and the client that have place in my application:

| Char | Server command          | Client command          | Message                              |
|------|-------------------------|-------------------------|--------------------------------------|
| m    | Any message             | Any message             | any message                          |
| f    | File to be saved        | File to be saved        | filename + '\n' + content            |
| r    | Request to download file|                         |                                      |
| x    | exit the room           | exit the room           |                                      |
| ?    | required answer         | required answer         | Question if save the file            |
| -    | disconnect              | disconnect              |                                      |
| s    |                         |                         | File was sent/saved (EOF)            |
| a    | Append to file          | Append to file          | filename + '\n' + content            |
| h    | End Of History (EOH)    | EOH, start messaging    |                                      |

This table provides a clear overview of the commands exchanged between the server and the client, along with the associated messages for each command.
#### And, generally the table of the sequence of events in my app
![ServerClient drawio (1)](https://github.com/LevkoBe/GroupChat/assets/118983753/0b25ac31-dcb8-42b1-a60d-ff97bf5edfbf)

### Now, let's take a look at how the functionality of the program was implemented.

#### Here I'll go over the next features:
1. Entering group;
2. Sending file;
3. Saving file;
4. Rejoining a group;
5. Sending messages;
6. Handling disconnections.

##### And, starting from the beginning:
#### Entering group
My implemeentation of the server works in the way it continuously waits for new users to connect, and whenever the new connection happens, separate thread is being created for specific client's socket. That thread has its own function handleClient, which is responsible for that User, handling its identification, entering a room, and receiving messages from that user:
```
// GroupChat.cpp
    while (server.serverSocket) { // connecting new clients
        SOCKET clientSocket = accept(server.serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            Common::errorMessage("Accept failed.\n");
            closesocket(server.serverSocket);
            WSACleanup();
            break;
        }

        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Client " << clientSocket << " connected.\n";
        clients.emplace_back(handleClient, clientSocket);
    }

// Server.cpp
void Server::handleClient(SOCKET clientSocket) {
    clientsMutex.lock();
    clients.push_back(clientSocket);
    clientsMutex.unlock();

    std::string username = askForUsername(clientSocket);
    std::shared_ptr<User> user = std::make_shared<User>(username, clientSocket);

    while (user->state != Disconnected) { // otherwise -- disconnect

        std::shared_ptr<Room> room = joinRoom(user);
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "Client " << clientSocket << " added to a group.\n";
        }
        receiveMessages(user, room);
    }
    clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
}
```
And basically, here we send a question to the user asking to provide their name (method _**askForUsername()**_), and when he does so, we later start a loop that works until user disconnects. Inside it we have two main components: _**joinRoom()**_ -- to ask for group name, and password (if needed), and adding the user to the group, on success.

On the client's side, we also have a continuous loop until disconnection with a few different states included:
```
    std::thread receiveThread(receiveMessages, std::ref(state));
    while (state != Disconnected) {
        if (state == Waiting)
            continue;
        else if (state == Entering)
            client.enterGroup(state);
        else if (!client.sendMessage(state))
            state = Disconnected;
    }
```
So, after identification step, with sending user's name, we start a thread for receiving messages. And, when the loop is entered, we check for the user's status. If it's 'Waiting', we wait for a little bit, untill user is added to a group. Then, Server sends history, followed be a message with option 'h', and we can proceed to sending messages by ourselves.

A little bit more about the receiving messages thread. It looks like that:
```
void Client::receiveMessages(std::atomic<State>& state) {
    char option;
    std::string message;
    while (state != Disconnected) {
        {
            std::unique_lock<std::mutex> lock(recvLocker);
            answerRequired.wait(lock, [ & ] { return (state != Entering); });
            option = Common::receiveOptionType(clientSocket);
            message = Common::receiveChunkedData(clientSocket);
        }
        switch (option) {
            case '?':
                print(message, 2, 4);
                state = AnswerRequired;
                break;
            case 'm':
                print(message, 2, 3);
                break;
            case 'f':
                Common::createFile(message);
                print("file is being downloaded...", 2, 4);
                break;
            case 'a':
                Common::appendToFile(message);
                break;
            case 's':
                print("file downloaded.", 3, 4);
                break;
            case 'h':
                state = Messaging;
                break;
            case 'x':
                state = Entering;
                break;
            case '-':
            default:
                state = Disconnected;
                std::cerr << "Server disconnected.\n";
                return;
                break;
        }
    }
}
```
And in case of different marks on messages from the server, it has different responses:
- on simple message (**m**), we print message in console;
- on required question (**?**), -- used when entering group, and responding Y/N for file sending from server -- we change state to AnswerRequired;
- on the first chunk of a file (**f**), we create a file, and notify a user in console);
- on the following file chunks (**a**), we append them to the already created file;
- on the message of EOF (**s**), we display message that the file was downloaded;
- on the message of exiting group, or Disconnecting (**x**/**-**), we change the state accordingly.

Sending messages has quite similar logic:
- If state is AnswerRequired, we require user to respond with 'y'/'n'.
- Otherwise, we check for special commands (**_file**/**_save**/**_exit**/**stop**), sending/receiving file, or changing the state in case it's needed.
- And if nothing of the beforementioned happens, we simply send the message (if it's not empty) to the server, and output it in console as well.

As I already mentioned previously, **sendFile()** method reads, and sends a file by chunks, marking the first one with **'f'**, others with **'a'**, and sending closing message with flag **'s'** to notify server that the whole file was sent, and can be send further to the users who wants to receive it.

##### Server's side:
As we took a look at the client, we can explore how the server receives and sends messages.

As you may remember from the upper sequence diagram, server, except of the main thread receiving new clients' connections, has N threads to receive messages from each user, and one more that sends messages, integrating Message Queue, that waits untill a message is added, and then broadcasts message to users in a specific group.

###### Starting from receiving messages:
Similarly to client's receiving thread, it checks flag with which the message is marked and behaves in accordance to what the expected response is.
The code is the following:
```
void Server::receiveMessages(std::shared_ptr<User> user, std::shared_ptr<Room> room) {
    std::string message;
    std::string path = "serverFolder\\" + user->room;
    std::shared_ptr<Message> userMessage = std::make_shared<Message>();
    while (user->state == InRoom) {
        char method = receiveSignal(user);
        message = Common::receiveChunkedData(user->clientSocket);
        switch (method) {
        case 'm':
            *userMessage = Message(message, user->username, user->clientSocket, roomByString(user->room));
            messenger.addMessageToQueue(userMessage);
            break;
        case 'f':
            Common::removeFolderContents(path); // server limitations:
            Common::createFile(message, path); // storing only the last sent file
            break;
        case 'a':
            Common::appendToFile(message, path);
            break;
        case 'r':
            message = Common::getFirstFile(path);
            if (message != "") {
                *userMessage = Message(message, user->username, user->clientSocket, room, FileRequest);
                messenger.addMessageToQueue(userMessage);
            }
            break;
        case 's':
            userMessage = std::make_shared<Message>(message, user->username, user->clientSocket, room, File);
            messenger.addMessageToQueue(userMessage);
            break;
        case 'x':
            message = "     User " + user->username + " left the group.     ";
            *userMessage = Message(message, user->username, user->clientSocket, room);
            messenger.addMessageToQueue(userMessage);

            Common::sendChunkedData(user->clientSocket, 'x', "You can join new room!");
            user->state = Connected;
            break;
        case '-':
        default:
            message = "     User " + user->username + " left the group.     ";
            *userMessage = Message(message, user->username, user->clientSocket, room);
            messenger.addMessageToQueue(userMessage);

            {
                std::scoped_lock<std::mutex> lock(room->roomLock);
                room->users.erase(std::remove(room->users.begin(), room->users.end(), user), room->users.end());
            }
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "Client " << user->clientSocket << " disconnected.\n";
            closesocket(user->clientSocket);
            break;
        }
    }
}
```
And, apart from the same logic of recognizing simple messages, first, and second file chunks, 'file sent' mark, and 'eXiting group' message, we also work with files in the folder of the group (created simultaneously with the group), and Message Queue, adding messages there if needed. 

Notice that when adding messages to the queue, we mark them in different ways:
- "Text" -- for simple text messages;
- "File" -- for received files;
- "FileRequest" -- for sending file to the user who requested.

In the MessageQueue class handling is the following:
- **broadcastMessages()** method is started when the group is created, and waits if there're no messages available;
```
void Messenger::broadcastMessages(std::mutex& consoleMutex) {
    while (true) {
        std::unique_lock<std::mutex> lock(messageQueueMutex);
        messageAvailableCondition.wait(lock, [&](){ return !messageQueue.empty(); });

        while (!messageQueue.empty()) {
            std::shared_ptr<Message> message = messageQueue.front();
            messageQueue.pop();
            broadcastMessage(*message, consoleMutex);
        }
    }
}
```
- **addMessageToQueue()** pushes message into queue, and notifies _broadcastMessage()_ about it:
```
void Messenger::addMessageToQueue(std::shared_ptr<Message> message) {
    {
        std::scoped_lock<std::mutex> lock(messageQueueMutex);
        messageQueue.push(message);
    }
    messageAvailableCondition.notify_one();
}
```
- And **broadcastMessage()** method considers the message type, and sends either simple message to all the members of the room, or question if to send the file, or the file itself:
```
void Messenger::broadcastMessage(const Message& message, std::mutex& consoleMutex) {

    std::shared_ptr<Room> room = message.room;
    std::string folderpath;

    {
        std::scoped_lock<std::mutex> lock(room->roomLock); // to avoid accessing removed users

        switch (message.type) {
            case Text:
                room->messageHistory.push_back(message.toStr());
                for (std::shared_ptr<User> user : room->users) {
                    if (user->clientSocket != message.senderSocket) {
                        Common::sendChunkedData(user->clientSocket, 'm', message.toStr());
                    }
                }
                break;
            case File:
                room->messageHistory.push_back(message.toStr());
                for (std::shared_ptr<User> user : room->users) {
                    if (user->clientSocket != message.senderSocket) {
                        Common::sendChunkedData(user->clientSocket, 'm', message.toStr());
                        Common::sendChunkedData(user->clientSocket, '?', "Do you want to download the file (Y/N)?");
                    }
                }
                break;
            case FileRequest:
                for (std::shared_ptr<User> user : room->users) {
                    if (user->clientSocket == message.senderSocket) {
                        folderpath = "serverFolder\\" + room->groupName;
                        Common::sendFile(user->clientSocket, message.message, folderpath);
                    }
                }
                break;
            default:
                break;
        }
    }
    std::scoped_lock<std::mutex> lock(consoleMutex);
    std::cout << "Client " << message.sender << ": " << message << std::endl;
}
```

#### These are the main features I decided to present about my program. Everything else you can see in the code, as the repo is publicly available to everyone.

### And, to showcase the work of the program, I'll present a few screenshots of its execution:

![2024-02-23](https://github.com/LevkoBe/GroupChat/assets/118983753/998441c9-82be-4bdd-8aca-a97e596156a7)
![2024-02-23 (1)](https://github.com/LevkoBe/GroupChat/assets/118983753/f9e2b710-cc20-48e2-ae1d-cf265e862feb)
![2024-02-23 (2)](https://github.com/LevkoBe/GroupChat/assets/118983753/cd4d358b-f802-4ad1-866b-e6b8b52fd03c)
![2024-02-23 (3)](https://github.com/LevkoBe/GroupChat/assets/118983753/158d3745-5a34-4c9b-b943-852ec4afa46a)
![2024-02-23 (4)](https://github.com/LevkoBe/GroupChat/assets/118983753/caf89884-84bf-437a-ae0e-be8ad600fc50)
![2024-02-23 (5)](https://github.com/LevkoBe/GroupChat/assets/118983753/ffaa83cf-e5f8-4d25-9c69-1ccb47f2448f)
![2024-02-23 (6)](https://github.com/LevkoBe/GroupChat/assets/118983753/5fddfe22-63e1-4710-b932-95911c11b8b7)




