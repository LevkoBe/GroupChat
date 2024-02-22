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
![image](https://github.com/LevkoBe/GroupChat/assets/118983753/79357ddd-66d7-4a29-8999-9a9266601975)

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
| h    | End Of History          |                         |                                      |

This table provides a clear overview of the commands exchanged between the server and the client, along with the associated messages for each command.
#### And, generally the table of the sequence of events in my app
![ServerClient drawio (1)](https://github.com/LevkoBe/GroupChat/assets/118983753/0b25ac31-dcb8-42b1-a60d-ff97bf5edfbf)
