#ifndef __AT_BASECMD_H
#define __AT_BASECMD_H

void at_exeCmdNull(uint8_t id);
void at_setupCmdE(uint8_t id, char *pPara);
void at_exeCmdRst(uint8_t id);
void at_exeCmdGmr(uint8_t id);

#endif
