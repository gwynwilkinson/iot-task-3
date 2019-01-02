#ifndef IOT_TASK_3_COMMUNICATION_H
#define IOT_TASK_3_COMMUNICATION_H

void decryptMessage(char* dpk);
void generateDPK(char dpk[21], char salt[], char PIN[4]);
void sendAck(int serviceID, int serviceCode, int status);

#endif //IOT_TASK_3_COMMUNICATION_H
