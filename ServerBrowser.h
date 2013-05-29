/*
 *  ServerBrowser.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdexcept>

/*!
 * @class ServerBrowser
 * @abstract Very crude server browser
 * @discussion This is used to find servers in a local network. Due to the
 * limited scope of this project, there are a couple of notable omissions. In
 * particular, it only stores information of one server. Additionally, it is not
 * aware of any changes in server status (most importantly server shutting down).
 */
class ServerBrowser
{
	bool lastServerIsIPv6;
#ifdef ANDROID_NDK
	struct __kernel_sockaddr_storage *lastServerAddress;
#else
	struct sockaddr_storage *lastServerAddress;
#endif
	
	int discoverySocketv4;
	int discoverySocketv6;
	int nfds;
	
	unsigned updatesSinceLastBroadcast;
	
	unsigned short portNumber;
	
	bool startBroadcastIPv4();
	bool startBroadcastIPv6();
	bool readPacketFrom(int socket);
	
public:
	/*!
	 * @abstract Creates the server browser
	 * @discussion Will try to look for servers both on IPv6 and IPv4, if the
	 * protocols are available.
	 * @param port Only servers that use this port will be found.
	 */
	ServerBrowser(unsigned short port);
	
	/*!
	 * @abstract Destructor
	 * @discussion Closes the server browser and stops looking for servers.
	 */
	~ServerBrowser();
	
	/*!
	 * @abstract Looks for incoming server messages and sends out own server
	 * search messages.
	 * @discussion This method looks whether any messages came in and sets
	 * the latest server based on that. It also sends out own messages if
	 * no server has been found yet.
	 */
	void update() throw(std::runtime_error);
	
	/*!
	 * @abstract Returns the address of the last server that was found.
	 * @discussion The address can be IPv4 or IPv6. Look at isLastServerIPv6()
	 * to determine which of these two is the protocol used. Between calls of
	 * update(), both will report consistent results.
	 * @result A sockaddr (either a sockaddr_in or sockaddr_in6). Do not delete
	 * it, thank you very much.
	 */
	const struct sockaddr *getLastServerAddress() const throw() { return reinterpret_cast<struct sockaddr *> (lastServerAddress); }
	
	/*! Was the last server found IPv6 or not? */
	bool isLastServerIPv6() const throw() { return lastServerIsIPv6; }
};
