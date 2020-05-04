#pragma once

/*
 * Resources:
 * http://www.infradead.org/~tgr/libnl/doc/core.html
 * http://www.infradead.org/~tgr/libnl/doc/api/group__core.html
 * http://www.infradead.org/~tgr/libnl/doc/api/group__genl.html
 * https://www.kernel.org/doc/html/v4.12/driver-api/80211/cfg80211.html
 * https://git.kernel.org/pub/scm/linux/kernel/git/jberg/iw.git
 */

#include <map>
#include <string>

namespace NetLink {

extern "C" {
#include <netlink/handlers.h>
}

struct scan_info {
  std::string bssid;
  std::string ssid;
  int last_seen;
  int freq;
  float signal;
  int channel;
};

struct link_result {
  bool link_found;
  uint8_t mac_addr[8];
  std::string bssid;
  std::string ssid;
  int freq;
  int channel;
};

struct station_info {
  float signal;
};

class Message {
  friend class Socket;

public:
  Message();
  ~Message();

  Message(const Message &that) = delete;
  Message &operator=(const Message &that) = delete;

  virtual int put(int family) const = 0;
  nl_recvmsg_msg_cb_t getCallback();

private:
  struct nl_msg *getRawMsg() const;

protected:
  struct nl_msg *msg;
  nl_recvmsg_msg_cb_t callback;
};

class MessageInterface : public Message {
public:
  MessageInterface();

  int put(int family) const;
  std::map<unsigned char, std::string> getInterfaces();
  int callback_interface(std::pair<unsigned char, std::string> interface);

private:
  std::map<unsigned char, std::string> interfaces;
};

class MessageScan : public Message {
public:
  MessageScan(signed long long devidx);

  int put(int family) const;
  std::map<std::string, scan_info> getScan();
  int callback_scan(scan_info scan);

private:
  std::map<std::string, scan_info> scans;
  signed long long m_devidx;
};

class MessageLink : public Message {
public:
  MessageLink(signed long long devidx);

  int put(int family) const;
  link_result getLink();
  int callback_link(link_result link);

private:
  link_result link;
  signed long long m_devidx;
};

class MessageStation : public Message {
public:
  MessageStation(signed long long devidx, link_result link);

  int put(int family) const;
  station_info getStation();
  int callback_station(station_info station);

private:
  station_info station;
  link_result link;
  signed long long m_devidx;
};

class Callback {
  friend class Socket;

public:
  Callback();
  ~Callback();

  Callback(const Callback &that) = delete;
  Callback &operator=(const Callback &that) = delete;

  void set_callback(Message *msg, nl_recvmsg_msg_cb_t callback, int *err);

  void setupError(nl_recvmsg_err_cb_t func, void *arg,
                  nl_cb_kind kind = NL_CB_CUSTOM);
  void setup(nl_cb_type type, nl_recvmsg_msg_cb_t func, void *arg,
             nl_cb_kind kind = NL_CB_CUSTOM);

private:
  struct nl_cb *getRawCb() const;

  struct nl_cb *cb;
};

class Socket {
public:
  Socket();
  ~Socket();

  Socket(const Socket &that) = delete;
  Socket &operator=(const Socket &that) = delete;

  void connect();
  int resolve(std::string);
  void set_cb(const Callback *s_cb);
  int send_auto(const Message *msg);
  void receiveMessage(const Callback *cb);

private:
  struct nl_sock *nl_sock;
};

class Nl80211 {
public:
  Nl80211();
  ~Nl80211();

  int wait();
  void sendMessage(Message *msg);
  int sendMessageWait(Message *msg);

private:
  Socket socket;
  Callback cb;
  Callback s_cb;
  int state;
};

} // namespace NetLink
