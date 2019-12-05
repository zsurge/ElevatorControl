#include "cJSON_Process.h"

#define LOG_TAG    "CJSON"
#include "elog.h"

/*******************************************************************
 *                          ��������                               
 *******************************************************************/



cJSON* cJSON_Data_Init(void)
{
  cJSON* cJSON_Root = NULL;    //json���ڵ�
  
  cJSON_Root = cJSON_CreateObject();   /*������Ŀ*/
  if(NULL == cJSON_Root)
  {
      return NULL;
  }
  cJSON_AddStringToObject(cJSON_Root, NAME, DEFAULT_NAME);  /*���Ԫ��  ��ֵ��*/
  cJSON_AddNumberToObject(cJSON_Root, TEMP_NUM, DEFAULT_TEMP_NUM);
  cJSON_AddNumberToObject(cJSON_Root, HUM_NUM, DEFAULT_HUM_NUM);
  
  char* p = cJSON_Print(cJSON_Root);  /*p ָ����ַ�����json��ʽ��*/
  
  log_i("send json = %s\r\n",p); 


  my_free(p);
  p = NULL;
  
  return cJSON_Root;
  
}
uint8_t cJSON_Update(const cJSON * const object,const char * const string,void *d)
{
  cJSON* node = NULL;    //json���ڵ�
  node = cJSON_GetObjectItem(object,string);
  if(node == NULL)
    return NULL;
  if(cJSON_IsBool(node))
  {
    int *b = (int*)d;
//    printf ("d = %d",*b);
    cJSON_GetObjectItem(object,string)->type = *b ? cJSON_True : cJSON_False;
  }
  else if(cJSON_IsString(node))
  {
    cJSON_GetObjectItem(object,string)->valuestring = (char*)d;
  }
  else if(cJSON_IsNumber(node))
  {
    double *num = (double*)d;
    cJSON_GetObjectItem(object,string)->valuedouble = (double)*num;

    my_free(num);
  }

   cJSON_Delete(node);

  return 1;

}

void Proscess(void* data)
{
    log_d("��ʼ����JSON����");
    cJSON *root,*json_name,*json_temp_num,*json_hum_num,*json_obj_data;
  
    root = cJSON_Parse((char*)data); //������json��ʽ

    if (root == NULL)                 // ���ת�������򱨴��˳�
    {
        log_d("1.Error before: [%s]\n", cJSON_GetErrorPtr());
        return ;
    }
      

  json_name = cJSON_GetObjectItem( root , (const char*)"commandCode");  //��ȡ��ֵ����
  
  json_obj_data = cJSON_GetObjectItem( root , "data" );
  
  json_temp_num = cJSON_GetObjectItem( json_obj_data , "currentModel" );

  json_hum_num = cJSON_GetObjectItem( root , "deviceCode" );

  log_d("code:%s ;currentModel:%d ;deviceCode:%s\r\n",
              json_name->valuestring,
              json_temp_num->valuestring,
              json_hum_num->valuestring);

  cJSON_Delete(root);  //�ͷ��ڴ� 
  cJSON_Delete(json_name);
  cJSON_Delete(json_obj_data);
  cJSON_Delete(json_temp_num);
  cJSON_Delete(json_hum_num);
}








