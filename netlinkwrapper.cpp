#include <system_error>

#include "helper.h"
#include "netlinkwrapper.h"

namespace NetLink {

extern "C" {
#include "netlink/genl/ctrl.h"
#include "netlink/genl/genl.h"
#include "netlink/netlink.h"
#include <linux/nl80211.h>
#include <net/if.h>
}

/*
 * @see iw/interface.c: print_iface_handler
 */
static int nl3_callback_interface(struct nl_msg *msg, void *arg) {
  struct genlmsghdr *gnlh =
      static_cast<genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0), NULL);

  if (tb_msg[NL80211_ATTR_IFNAME] && tb_msg[NL80211_ATTR_IFINDEX]) {
    unsigned char idx = nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]);
    std::string ifname = nla_get_string(tb_msg[NL80211_ATTR_IFNAME]);
    std::pair<unsigned char, std::string> interface({idx, ifname});

    MessageInterface *m = static_cast<MessageInterface *>(arg);
    return m->callback_interface(interface);
  }
  return NL_SKIP;
}

/*
 * @see iw/scan.c: print_bss_handler
 */
static int nl3_callback_scan(struct nl_msg *msg, void *arg) {
  struct scan_info scan;

  // struct genlmsghdr *gnlh = genlmsg_hdr(nlmsg_hdr(msg));
  struct genlmsghdr *gnlh =
      static_cast<genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct nlattr *bss[NL80211_BSS_MAX + 1];

  static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {};
  bss_policy[NL80211_BSS_FREQUENCY].type = NLA_U32;
  bss_policy[NL80211_BSS_BSSID] = {};
  bss_policy[NL80211_BSS_INFORMATION_ELEMENTS] = {};
  bss_policy[NL80211_BSS_SIGNAL_MBM].type = NLA_U32;
  bss_policy[NL80211_BSS_SEEN_MS_AGO].type = NLA_U32;

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0), NULL);
  if (nla_parse_nested(bss, NL80211_BSS_MAX, tb_msg[NL80211_ATTR_BSS],
                       bss_policy)) {
    return NL_SKIP;
  }

  if (!tb_msg[NL80211_ATTR_BSS]) {
    return NL_SKIP;
  }

  scan.bssid =
      mac_addr(static_cast<unsigned char *>(nla_data(bss[NL80211_BSS_BSSID])));
  scan.freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
  scan.signal =
      static_cast<int>(nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM])) / 100.0;
  scan.last_seen = static_cast<int>(nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]));

  int ds_channel = -1;
  int hd_channel = -1;
  int hd_channel_offset = -1;
  int hd_channel_width = -1;
  int vhd_channel_width = -1;
  int vhd_center_freq_1 = -1;
  int vhd_center_freq_2 = -1;

  if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
    uint8_t *ie =
        static_cast<uint8_t *>(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]));
    int ielen = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
    // nla_get_string()

    while (ielen >= 2 && ielen >= ie[1]) {
      uint8_t type = ie[0];
      uint8_t len = ie[1];
      uint8_t *data = ie + 2;

      switch (type) {
      case 0: /* SSID */
        if (len > 0 && len <= 32)
          scan.ssid = std::string(reinterpret_cast<char *>(data), len);
        break;
      case 3: /* DS channel */
        if (len == 1)
          ds_channel = *data;
        break;
      case 61: /* HT operation */
        hd_channel = data[0];
        hd_channel_offset = (data[1] & 0x3);
        hd_channel_width = ((data[1] & 0x4) >> 2);
        break;
      case 192: /* VHT operation */
        vhd_channel_width = data[0];
        vhd_center_freq_1 = data[1];
        vhd_center_freq_2 = data[2];
        break;
      }

      ielen -= len + 2;
      ie += len + 2;
    }
  }

  scan.channel = ds_channel;
  if (ds_channel == -1) {
    scan.channel = hd_channel;
  }

  MessageScan *m = static_cast<MessageScan *>(arg);
  return m->callback_scan(scan);
}

/*
 * @see iw/link.c: link_bss_handler
 */
static int nl3_callback_link_bss(struct nl_msg *msg, void *arg) {
  struct link_result link;

  struct genlmsghdr *gnlh =
      static_cast<genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct nlattr *bss[NL80211_BSS_MAX + 1];

  static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {};
  bss_policy[NL80211_BSS_FREQUENCY].type = NLA_U32;
  bss_policy[NL80211_BSS_BSSID] = {};
  bss_policy[NL80211_BSS_INFORMATION_ELEMENTS] = {};
  bss_policy[NL80211_BSS_STATUS].type = NLA_U32;

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0), NULL);
  if (nla_parse_nested(bss, NL80211_BSS_MAX, tb_msg[NL80211_ATTR_BSS],
                       bss_policy)) {
    return NL_SKIP;
  }

  if (!bss[NL80211_BSS_BSSID]) {
    return NL_SKIP;
  }

  if (!bss[NL80211_BSS_STATUS]) {
    return NL_SKIP;
  }

  if (nla_get_u32(bss[NL80211_BSS_STATUS]) != NL80211_BSS_STATUS_ASSOCIATED)
    return NL_SKIP;

  link.link_found = true;
  memcpy(link.mac_addr, nla_data(bss[NL80211_BSS_BSSID]), 6);

  link.bssid =
      mac_addr(static_cast<unsigned char *>(nla_data(bss[NL80211_BSS_BSSID])));
  link.freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);

  if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
    uint8_t *ie =
        static_cast<uint8_t *>(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]));
    int ielen = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);

    while (ielen >= 2 && ielen >= ie[1]) {
      uint8_t type = ie[0];
      uint8_t len = ie[1];
      uint8_t *data = ie + 2;

      switch (type) {
      case 0: /* SSID */
        if (len > 0 && len <= 32)
          link.ssid = std::string(reinterpret_cast<char *>(data), len);
        break;
      case 3: /* DS channel */
        if (len == 1)
          link.channel = *data;
        break;
      }

      ielen -= len + 2;
      ie += len + 2;
    }
  }

  MessageLink *m = static_cast<MessageLink *>(arg);
  return m->callback_link(link);

  return NL_SKIP;
}

/*
 * @see iw/link.c: print_link_sta
 */
static int nl3_callback_station(struct nl_msg *msg, void *arg) {
  struct station_info station;

  struct genlmsghdr *gnlh =
      static_cast<genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];

  static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {};
  stats_policy[NL80211_STA_INFO_SIGNAL].type = NLA_U8;

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0), NULL);
  if (!tb_msg[NL80211_ATTR_STA_INFO]) {
    return NL_SKIP;
  }

  if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
                       tb_msg[NL80211_ATTR_STA_INFO], stats_policy)) {
    return NL_SKIP;
  }

  station.signal =
      static_cast<int8_t>(nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]));

  MessageStation *m = static_cast<MessageStation *>(arg);
  return m->callback_station(station);

  return NL_SKIP;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
                         void *arg) {
  int *ret = static_cast<int *>(arg);
  *ret = err->error;
  return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg) {
  int *ret = static_cast<int *>(arg);
  *ret = 0;
  return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg) {
  int *ret = static_cast<int *>(arg);
  *ret = 0;
  return NL_STOP;
}

Message::Message() : msg(nullptr) {
  msg = nlmsg_alloc();
  if (!msg)
    throw std::system_error(std::make_error_code(std::errc::not_enough_memory),
                            "failed to allocate netlink message.");
}

Message::~Message() { nlmsg_free(msg); }

struct nl_msg *Message::getRawMsg() const {
  return msg;
}

nl_recvmsg_msg_cb_t Message::getCallback() { return callback; }

MessageInterface::MessageInterface() : Message() {
  callback = nl3_callback_interface;
}

int MessageInterface::put(int family) const {
  genlmsg_put(msg, 0, 0, family, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);
  return 0;
}

std::map<unsigned char, std::string> MessageInterface::getInterfaces() {
  return interfaces;
}

int MessageInterface::callback_interface(
    std::pair<unsigned char, std::string> interface) {
  interfaces[interface.first] = interface.second;
  return NL_SKIP;
}

MessageScan::MessageScan(signed long long devidx) : Message() {
  callback = nl3_callback_scan;
  m_devidx = devidx;
}

int MessageScan::put(int family) const {
  genlmsg_put(msg, 0, 0, family, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
  return nla_put_u32(msg, NL80211_ATTR_IFINDEX, m_devidx);
}

std::map<std::string, scan_info> MessageScan::getScan() { return scans; }

int MessageScan::callback_scan(struct scan_info scan) {
  scans[scan.bssid] = scan;
  return NL_SKIP;
}

Callback::Callback() : cb(nullptr) {
  cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (!cb)
    throw std::system_error(std::make_error_code(std::errc::not_enough_memory),
                            "failed to allocate netlink callback.");
}

Callback::~Callback() { nl_cb_put(cb); }

struct nl_cb *Callback::getRawCb() const {
  return cb;
}

void Callback::setupError(nl_recvmsg_err_cb_t func, void *arg,
                          enum nl_cb_kind kind) {
  nl_cb_err(cb, kind, func, arg);
}

void Callback::setup(enum nl_cb_type type, nl_recvmsg_msg_cb_t func, void *arg,
                     enum nl_cb_kind kind) {
  nl_cb_set(cb, type, kind, func, arg);
}

Socket::Socket() : nl_sock(nullptr) {
  nl_sock = nl_socket_alloc();
  if (!nl_sock) {
    throw std::system_error(std::make_error_code(std::errc::not_enough_memory),
                            "Failed to allocate netlink socket.");
  }
}

Socket::~Socket() { nl_socket_free(nl_sock); }

void Socket::connect() {
  if (genl_connect(nl_sock) < 0) {
    throw std::system_error(std::make_error_code(std::errc::no_link),
                            "Failed to connect to generic netlink.");
  }

  nl_socket_set_buffer_size(nl_sock, 8192, 8192);

  int err = 1;
  /* try to set NETLINK_EXT_ACK to 1, ignoring errors */
  setsockopt(nl_socket_get_fd(nl_sock), SOL_NETLINK, NETLINK_EXT_ACK, &err,
             sizeof(err));
}

int Socket::resolve(std::string name) {
  int family_id = genl_ctrl_resolve(nl_sock, name.c_str());
  if (family_id < 0) {
    throw std::system_error(
        std::make_error_code(std::errc::no_such_file_or_directory),
        "nl80211 not found."); // FIXME name
  }
  return family_id;
}

void Socket::set_cb(const Callback *s_cb) {
  nl_socket_set_cb(nl_sock, s_cb->getRawCb());
}

int Socket::send_auto(const Message *msg) {
  return nl_send_auto(nl_sock, msg->getRawMsg());
}

void Socket::receiveMessage(const Callback *cb) {
  nl_recvmsgs(nl_sock, cb->getRawCb());
}

Nl80211::Nl80211() : state(1) { socket.connect(); }
Nl80211::~Nl80211() {}

int Nl80211::wait() {
  while (state > 0)
    socket.receiveMessage(&cb);
  return state;
}

void Nl80211::sendMessage(Message *msg) {

  int family_id = socket.resolve("nl80211");

  if (msg->put(family_id) < 0)
    return;

  socket.set_cb(&s_cb);

  if (socket.send_auto(msg) < 0)
    return;

  state = 1;
  cb.setupError(error_handler, &state);
  cb.setup(NL_CB_FINISH, finish_handler, &state);
  cb.setup(NL_CB_ACK, ack_handler, &state);
  cb.setup(NL_CB_VALID, msg->getCallback(), msg);
}

int Nl80211::sendMessageWait(Message *msg) {
  sendMessage(msg);
  return wait();
}

MessageLink::MessageLink(signed long long devidx) {
  callback = nl3_callback_link_bss;
  m_devidx = devidx;
  link.link_found = false;
}

int MessageLink::put(int family) const {
  genlmsg_put(msg, 0, 0, family, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
  return nla_put_u32(msg, NL80211_ATTR_IFINDEX, m_devidx);
}

link_result MessageLink::getLink() { return link; }

int MessageLink::callback_link(link_result link) {
  this->link = link;
  return NL_SKIP;
}

MessageStation::MessageStation(signed long long devidx, link_result link) {
  callback = nl3_callback_station;
  this->link = link;
  station.signal = 0;
  m_devidx = devidx;
}

int MessageStation::put(int family) const {
  genlmsg_put(msg, 0, 0, family, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0);
  nla_put_u32(msg, NL80211_ATTR_IFINDEX, m_devidx);
  return nla_put(msg, NL80211_ATTR_MAC, 6, link.mac_addr);
}

station_info MessageStation::getStation() { return station; }

int MessageStation::callback_station(station_info station) {
  this->station = station;
  return NL_SKIP;
}

} // namespace NetLink
