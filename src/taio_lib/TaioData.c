#include "TaioData.h"

void PrintData(TaioData* data){
    printf("Printing data families data: \n");
    printf("Printing first family: \n");
    PrintSetFamily(data->Family1);
    printf("Printing second family: \n");
    PrintSetFamily(data->Family2);

}