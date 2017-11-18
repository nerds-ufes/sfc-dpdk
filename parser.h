#ifndef PARSER_H_
#define PARSER_H_

int parse_portmask(const char *portmask);

enum sfcapp_type parse_apptype(const char *type);

int parse_uint8(const char *str, uint8_t *res, int radix);

int parse_uint16(const char *str, uint16_t *res, int radix);

int parse_uint32(const char* str, uint32_t *res, int radix);

int parse_uint64(const char* str, uint64_t *res, int radix);

int parse_ether(const char *str, struct ether_addr *eth_addr);

int parse_ipv4(const char *str, uint32_t *ipv4);

void parse_config_file(char* cfg_filename);

#endif /* PARSER_H_ */