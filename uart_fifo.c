#include <stdio.h>
#include <conio.h>

#define CZ_OFF   0
#define CZ_ON    1
#define CZ_NULL  0

#define CDBG_CMN_SEND_BUFF_SIZE (2048)  /**< Size of data buffer */ 
#define CDBG_CMN_PRINT_HEX_MAX  (8)    /**< Print size of hexadecimal value */

typedef unsigned char UCHR;
typedef unsigned short USHT;
typedef unsigned long ULNG;

typedef struct {
  USHT rBuffSize;  /* buffer size   */
  UCHR *pBuffer;   /* buffer        */
  USHT rWritePtr;  /* write pointer */
  USHT rReadPtr;   /* read_pointer  */
}ST_DBG_CMN_RING_BUFF, *PST_DBG_CMN_RING_BUFF;


static ST_DBG_CMN_RING_BUFF stDbgLogBuff;  /**< Send buffer structure */  /* @@DEBUG_CMD */
static USHT rDbgLogCycintCount1ms;  /**< Count value for Time stamp */
static UCHR rDbgLogBuff[CDBG_CMN_SEND_BUFF_SIZE];  /**< Send buffer */  /* @@DEBUG_CMD */

static UCHR rDbgLogSendingflg;
static UCHR rDbgLogSendEndflg;

const static UCHR tDbgCmnConvertTable[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};  /**< Table for convert to character */ /* @@DEBUG_CMD */

/** **************************************************************************
  @brief         Initial setting of the ring buffer.
  @param[in]     *pHead  Appoint the top address of the ring buffer.
 *****************************************************************************/
void DbgCmnRingBuffInit(ST_DBG_CMN_RING_BUFF *pHead)
{
  (pHead->rWritePtr) = (pHead->rReadPtr); /* Ring buffer statement become empty. */
}

/** **************************************************************************
  @brief         Initial setting of the log output.
  @attention     Before log output functions called, call this function.
                 This function clear the log send buffer,
                 and clear the counter for time stamp.
 *****************************************************************************/
void DbgLogInit(void)
{

  /* Initial setting of send buffer */
  stDbgLogBuff.rBuffSize = (USHT)CDBG_CMN_SEND_BUFF_SIZE;
  stDbgLogBuff.pBuffer   = rDbgLogBuff;
  
  DbgCmnRingBuffInit(&stDbgLogBuff);
  stDbgLogBuff.rWritePtr = (USHT)0;
  stDbgLogBuff.rReadPtr  = (USHT)0;

  /* Initial setting of counter for Time stamp */
  rDbgLogCycintCount1ms = (USHT)0;
  
  rDbgLogSendEndflg = (UCHR)CZ_OFF;
  rDbgLogSendingflg = (UCHR)CZ_OFF;
  
}


/** **************************************************************************
  @brief         Get the statement of the ring buffer, full or not full.
  @param[in]     *pHead  Appoint the top address of the ring buffer.
  @return        The result that buffer is full or not full.
                 CZ_ON: Buffer is full, CZ_OFF: Buffer is not full. 
 *****************************************************************************/
UCHR DbgCmnRingBuffCheckFull(ST_DBG_CMN_RING_BUFF *pHead)
{
  UCHR rRet = (UCHR)CZ_OFF;
  USHT rWritePtrNext;    /* Next write point */

  rWritePtrNext = ((pHead->rWritePtr) +(USHT)1)%(pHead->rBuffSize);

  if (rWritePtrNext == (pHead->rReadPtr)){
    rRet = (UCHR)CZ_ON;  /* Next write point equal current read point, 
                            the ring buffer statement is full.         */
  }

  return rRet;
}

/** **************************************************************************
  @brief         Get the statement of the ring buffer, empty or not empty.
  @param[in]     *pHead  Appoint the top address of the ring buffer.
  @return        The result that buffer is empty or not empty.
                 CZ_ON: Buffer is empty, CZ_OFF: Buffer is not empty.
 *****************************************************************************/
UCHR DbgCmnRingBuffCheckEmpty(ST_DBG_CMN_RING_BUFF *pHead)
{
  UCHR rRet = (UCHR)CZ_OFF;

  if ((pHead->rWritePtr) == (pHead->rReadPtr)){
    rRet = (UCHR)CZ_ON;  /* When write point equal read point, ring buffer statement is empty. */
  }

  return rRet;

}

/****************************************************************************
  @brief         Write 1 byte data to ring buffer.
  @param[in]     *pHead  Appoint the top address of the ring buffer.
  @param[in]     rInputData  Appoint the write data.
 *****************************************************************************/
void DbgCmnRingBuffPut(ST_DBG_CMN_RING_BUFF *pHead, UCHR rInputData)
{
  if ((UCHR)CZ_ON != DbgCmnRingBuffCheckFull(pHead)) {
    pHead->pBuffer[pHead->rWritePtr] = rInputData;  /* Write a data to ring buffer */
    (pHead->rWritePtr) = ((pHead->rWritePtr)+(USHT)1)%(pHead->rBuffSize);  /* Write point ++ */
  }
}

/****************************************************************************
  @brief         Read 1 byte data from the ring buffer.
  @param[in]     *pHead  Appoint the top address of the ring buffer.
  @return        Read data from the ring buffer.
 *****************************************************************************/
UCHR DbgCmnRingBuffGet(ST_DBG_CMN_RING_BUFF *pHead)
{
  UCHR rOutputData = (UCHR)0;

  if ((UCHR)CZ_ON != DbgCmnRingBuffCheckEmpty(pHead)) {
    rOutputData = pHead->pBuffer[pHead->rReadPtr];  /* Read a data from ring buffer */
    (pHead->rReadPtr) = ((pHead->rReadPtr)+(USHT)1)%(pHead->rBuffSize);  /* Read point ++ */
  }
  
  return rOutputData;
}


/** **************************************************************************
  @brief         Store send data in send buffer.
  @attention     Don't appoint 0 to param[in] rSize.
  @pre           Include "io_uart.h", "dbg_sys.h".
  @param[in]     pString[] Appoint character strings that store on buffer.
  @param[in]     rSize     Appoint length character strings.
 *****************************************************************************/
void DbgLogWriteCharacter(UCHR /*far*/ pString[], UCHR rSize)
{
  UCHR rLoop;
  UCHR rFullCheck = (UCHR)CZ_OFF;
  UCHR rSendData;

  for(rLoop=(UCHR)0; rLoop<rSize; rLoop++){
    /* Buffer full check */
    rFullCheck = DbgCmnRingBuffCheckFull(&stDbgLogBuff);  /* @@DEBUG_CMD */

    if(rFullCheck == (UCHR)CZ_ON){
      /* Buffer is full */
      break;
    }
    else{
      /* Buffer is not full */
      DbgCmnRingBuffPut(&stDbgLogBuff, pString[rLoop]);  /* @@DEBUG_CMD */
    }
  }
  
#if 0
  if( rDbgLogSendEndflg == CZ_ON ){
    rDbgLogSendEndflg = (UCHR)CZ_OFF;
    rDbgLogSendingflg = (UCHR)CZ_OFF;
  }
  
  /* UART send status check */
  if ((UCHR)CZ_OFF == rDbgLogSendingflg){
    rDbgLogSendingflg = (UCHR)CZ_ON;
    /* Finished sending */
    rSendData = DbgCmnRingBuffGet(&stDbgLogBuff);  /* Get send data from Send buffer */
    DbgLogSendByte(rSendData);  /* Send data to UART */
  }
  else{
    /* Sending... */
    /* No operation */
  }
#endif

}

/** **************************************************************************
  @brief         Display character strings of the option to screen which connected UART.
  @attention     Displayable number of the letters depend the size of log send buffer.
                 Log send buffer is settable. Prease change value CDBG_CMN_SEND_BUFF_SIZE in dbg_cmn_func.h
  @pre           Before run this function, confirm to conplete setting of UART that you will use,
                 and the UART started already.
  @param[in]     pString[] Appoint character strings that display at screen.
  @param[in]     rSize     Appoint length of character strings that display at screen.
                           In the case that over displayable number of the letters, displayed 
                           displayable number of the letters only.
 *****************************************************************************/
void DbgLogPrintStr(UCHR /*far*/ pString[], UCHR rSize)
{
  if ((UCHR)0 != rSize) {
    DbgLogWriteCharacter(pString, rSize);  /* Write data to send buffer. if uart is not used, start send data */
  }
}

/** **************************************************************************
  @brief         Convert unsigned integer value to character strings that form is hexadecimal.
  @attention     Figures of character strings is 8 figures.
                 The character '0' write at brank figure.
  @param[in]     rHexData   Appoint the unsigned integer value that is converted to character strings.
  @param[in]     rSize      Appoint the length that display character strings.
  @param[in]     pStrData[] Appoint the address of store buffer for character strings after conversion.
  @return        Number of the letters of character strings after conversion.
 *****************************************************************************/
UCHR DbgCmnConvertHexToStr(ULNG rHexData, UCHR rSize, UCHR pStrData[])
{
  UCHR rLoop;
  UCHR rTemp = (UCHR)CZ_NULL;

  if ((UCHR)8 < rSize){
    rSize = (UCHR)8;  /* Print figure maximum size is 8 */
  }

  /* Hexadecimal -> Character strings */	 
  for (rLoop=(UCHR)0; rLoop<(UCHR)CDBG_CMN_PRINT_HEX_MAX; rLoop++){
    pStrData[rLoop] = tDbgCmnConvertTable[(UCHR)(rHexData%((ULNG)16))];
    rHexData /= (ULNG)16;
  }
 
  /* Current, line up reversely. Change the place of character. */
  for (rLoop=(UCHR)0; rLoop<(rSize/((UCHR)2)); rLoop++){
    rTemp = pStrData[rLoop];
    pStrData[rLoop] = pStrData[rSize-rLoop-(UCHR)1];
    pStrData[rSize-rLoop-(UCHR)1] = rTemp;
  }

  return rSize;
}

/** **************************************************************************
  @brief         Display unsigned hexadecimal integer value of the option to screen which connected UART.
  @attention     Displayable number of the letters one time, 8 figures.
                 Display by right justify, assume one figure is one character.
                 Display form is '0'-'9', 'A'-'F'. The character '0' write at brank figure.
  @pre           Before run this function, confirm to conplete setting of UART that you will use,
                 and the UART started already.
  @param[in]     rHexData Appoint value that display on screen.
                          Appointable area of value is (0x00000000) - (0xFFFFFFFF).
  @param[in]     rSize    Appoint figures of display value.
                          if 9 figures are appointed, display 8 figures on screen.
 *****************************************************************************/
void DbgLogPrintHex(ULNG rHexData, UCHR rSize)
{
  UCHR rStrData[CDBG_CMN_PRINT_HEX_MAX];  /* Storing buffer after exchange value */
  UCHR rSizeStr;  /* String Size(length) after exchange value */

  if ((UCHR)0 != rSize) {
    rSizeStr = DbgCmnConvertHexToStr(rHexData, rSize, rStrData);  /* Convert from hexadecimal value to character strings */

    DbgLogWriteCharacter(rStrData, rSizeStr);   /* Output character strings */
  } 
}

/** **************************************************************************
  @brief         Handler of send interrupt(log function)
 *****************************************************************************/
void DbgLogTxIntr(void)
{
  UCHR rSendData;

  if ((UCHR)CZ_ON != DbgCmnRingBuffCheckEmpty(&stDbgLogBuff)){
    rSendData = DbgCmnRingBuffGet(&stDbgLogBuff);  /* Get send data from Send buffer */
    //DbgLogSendByte(rSendData);  /* Send data from Send buffer */
  }
  else{
    rDbgLogSendEndflg = (UCHR)CZ_ON;
  }

}

int main(){
  DbgLogInit();
  DbgLogPrintStr("ItemSetNotify",sizeof("ItemSetNotify"));
  while(!DbgCmnRingBuffCheckEmpty(&stDbgLogBuff)){
    printf("%c",DbgCmnRingBuffGet(&stDbgLogBuff));
  }

  getch();
  return 0;
}
