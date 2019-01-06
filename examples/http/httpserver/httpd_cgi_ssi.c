/**
  ******************************************************************************
  * @file    httpd_cg_ssi.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Webserver SSI and CGI handlers
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/


#include "lwip/opt.h"
#include "lwip/apps/httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "main.h"

#include <string.h>
#include <stdlib.h>

tSSIHandler ADC_Page_SSI_Handler;
uint32_t ADC_not_configured=1;

/* we will use character "t" as tag for CGI */
char const* TAGCHAR="t";
char const** TAGS=&TAGCHAR;

/* CGI handler for LED control */ 
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

/* Html request for "/leds.cgi" will start LEDS_CGI_Handler */
const tCGI LEDS_CGI={"/leds.cgi", LEDS_CGI_Handler};

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[1];


/**
  * @brief  ADC_Handler : SSI handler for ADC page 
  */
u16_t ADC_Handler(int iIndex, char *pcInsert, int iInsertLen)
{
  /* We have only one SSI handler iIndex = 0 */
  if (iIndex ==0)
  {  
     /* configure ADC if not yet configured
     if (ADC_not_configured ==1)       
     {
        ADC_Configuration();
        ADC_not_configured=0;
     } */
     
     /* get ADC conversion value */
	 extern uint16_t ADC_GetConversionValue(uint8_t adc_in_nr);
     uint16_t ADCVal = ADC_GetConversionValue(0);
     
     /* convert to Voltage,  step = 0.8 mV */
     ADCVal = (uint32_t)(ADCVal * 0.8);  
     
     /* get digits to display */
     char Digit1= ADCVal/1000;
     char Digit2= (ADCVal-(Digit1*1000))/100 ;
     char Digit3= (ADCVal-((Digit1*1000)+(Digit2*100)))/10;
     char Digit4= ADCVal -((Digit1*1000)+(Digit2*100)+ (Digit3*10));
        
     /* prepare data to be inserted in html */
     *pcInsert       = (char)(Digit1+0x30);
     *(pcInsert + 1) = (char)(Digit2+0x30);
     *(pcInsert + 2) = (char)(Digit3+0x30);
     *(pcInsert + 3) = (char)(Digit4+0x30);
    
    /* 4 characters need to be inserted in html*/
    return 4;
  }
  return 0;
}

/**
  * @brief  CGI handler for LEDs control 
  */
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  /* We have only one SSI handler iIndex = 0 */
  if (iIndex==0)
  {
    /* All leds off */
    LEDOff(1);
    LEDOff(2);
    LEDOff(3);
    //LEDOff(4);
    
    /* Check cgi parameter : example GET /leds.cgi?led=2&led=4 */
    for (int i=0; i<iNumParams; i++)
    {
      /* check parameter "led" */
      if (strcmp(pcParam[i] , "led")==0)   
      {
        /* switch led1 ON if 1 */
        if(strcmp(pcValue[i], "1") ==0) 
          LEDOn(1);
          
        /* switch led2 ON if 2 */
        else if(strcmp(pcValue[i], "2") ==0) 
          LEDOn(2);
        
        /* switch led3 ON if 3 */
        else if(strcmp(pcValue[i], "3") ==0) 
          LEDOn(3);
        
        /* switch led4 ON if 4
        else if(strcmp(pcValue[i], "4") ==0) 
          STM_EVAL_LEDOn(LED4); */
      }
    }
  }
  /* uri to send after cgi call*/
  return "/LED.html";  
}

/**
 * Initialize SSI handlers
 */
void httpd_ssi_init(void)
{  
  /* configure SSI handlers (ADC page SSI) */
  http_set_ssi_handler(ADC_Handler, (char const **)TAGS, 1);
  PRINTF("ADC SSI handler attached.\n");
}

/**
 * Initialize CGI handlers
 */
void httpd_cgi_init(void)
{ 
  /* configure CGI handlers (LEDs control CGI) */
  CGI_TAB[0] = LEDS_CGI;
  http_set_cgi_handlers(CGI_TAB, 1);
  PRINTF("LED CGI handler attached.\n");
}

void httpd_cgi_handler(struct fs_file *file, const char* uri, int iNumParams,
                              char **pcParam, char **pcValue
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
                                     , void *connection_state
#endif /* LWIP_HTTPD_FILE_STATE */
                                     ) {}
									 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
