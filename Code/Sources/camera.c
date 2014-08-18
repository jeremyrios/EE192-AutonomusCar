/********************************************/
/*      Camera                                                                  */
/*                                              */
/*                                              */
/********************************************/
#include "camera.h"

volatile uint16_t Result[128];
volatile uint8_t lineADC[128];
volatile uint8_t pixel;
volatile uint32_t i,j;
volatile uint32_t exposureTime, baseExposureTime;	//us
volatile uint16_t dynamicThreshold;
volatile uint16_t whiteThreshold,blackThreshold;
volatile uint8_t faultyLine;
uint8_t cameraState;
volatile float actualContrast;

extern buffer_t* out;
extern uint8_t line_middle;

int midMag = 0;
int pMidMag = 0;
int ppMidMag = 0;

void initCamera()
{
        pixel = 0;
        SIU.PCR[SI].R = 0x0200;                                 /* Program the Sensor read start pin as output*/
        SIU.PCR[CLK].R = 0x0200;                                /* Program the Sensor Clock pin as output*/
        baseExposureTime = INIT_EXPOSURE_TIME;
        exposureTime = 4000;//835;//19000;//5800;//5200 @6:30pm // INIT_EXPOSURE_TIME;// 
        actualContrast = 0;
        cameraState = STATE_READY;//  STATE_INITIALIZING;// 
        dynamicThreshold = THRESHOLD;
        whiteThreshold = 0;
        blackThreshold = 0;
        faultyLine = 0;
}

void EOC_ISR(void)
{
        //TransmitData("****Line Sensor Test****\n\r");
        uint16_t adcdata;
        float voltage;
        
        adcdata = ADC.CDR[0].B.CDATA;

#ifdef LINE_BLACK
        voltage = (float)5000*adcdata;
#endif
        
#ifdef LINE_WHITE
        voltage = (float)(5000*(0x03FF-adcdata));
#endif
        lineADC[pixel] = (uint8_t)(adcdata>>2);
        Result[pixel] = (uint16_t)(voltage/0x03FF);
//      fifo_write(&out->fifo,&adcdata,2);
        pixel++;
        
        for(i=0;i<DELAY;i++);   /* delay */
        
        //SIU.PGPDO[0].R &= ~0x00000004;        /* Sensor Clock Low */
        SIU.GPDO[CLK].R = 0;
        
        for(i=0;i<DELAY;i++);   /* delay */

        if(pixel<128)
        {
//              SIU.PGPDO[0].R |= 0x00000004;   /* Sensor Clock High */
                SIU.GPDO[CLK].R = 1;
                
                ADC.MCR.B.NSTART=1;                     /* Trigger normal conversions for ADC0 */
        }
        else
        {
//              SIU.PGPDO[0].R |= 0x00000004;   /* Sensor Clock High */
                SIU.GPDO[CLK].R = 1;
                
                for(i=0;i<50;i++);      /* delay */
                
//              SIU.PGPDO[0].R &= ~0x00000004;  /* Sensor Clock Low */
                SIU.GPDO[CLK].R = 0;
                
                for(i=0;i<50;i++);      /* delay */
                
                if(cameraState == STATE_READY)
                {
                        INTC.SSCIR[4].R = 2;      /*  then invoke software interrupt 4 */
                }
                else
                {
                        FindBestContrast();
                        if((exposureTime>LOWER_EXPOSURE_TIME))//&&(actualContrast<MINIMUM_CONTRAST))
                        {
                                exposureTime -= EXPOSURE_STEP;
                        }
                        else
                        {
                                exposureTime = baseExposureTime;
                                cameraState = STATE_READY;
                        }
                }
        }
        ADC.ISR.B.EOC = 1; /*clear interrupt flag*/
        ADC.CEOCFR[0].R = 0x00000002;
}

void newLine()
{
        TriggerCamera();
        
//      SIU.PGPDO[0].R |= 0x00000004;   /* Sensor Clock High */
//      SIU.GPDO[CLK].R = 1;
        
        pixel = 0;
        ADC.MCR.B.NSTART=1;                     /* Trigger normal conversions for ADC0 */
}

void initADC(void) {
        //ADC.MCR.R = 0x20020000;               /* Initialize ADC scan mode*/
        ADC.MCR.R = 0x00000000;                 /* Initialize ADC one shot mode*/
        ADC.NCMR[0].R = 0x00000007;             /* Select ANP1 inputs for normal conversion */
        INTC.PSR[62].R = 0x04;
        ADC.CTR[0].R = 0x00008606;              /* Conversion times for 32MHz ADClock */
        ADC.IMR.R    = 0x00000002;                      /* enable interrupt for end-of-conversion */
        ADC.CIMR[0].R = 0x00000002;                     /* select which channel will generate the interrupt */

}

void TriggerCamera()
{
        SIU.PCR[SI].R = 0x0200;                 /* Program the Sensor read start pin as output*/
        SIU.PCR[CLK].R = 0x0200;                /* Program the Sensor Clock pin as output*/
//      SIU.PGPDO[0].R &= ~0x00000014;          /* All port line low */
        SIU.GPDO[SI].B.PDO = 0;
        SIU.GPDO[CLK].B.PDO = 0;
        
        for(i=0;i<DELAY;i++);   /* delay */
//      SIU.PGPDO[0].R |= 0x00000010;           /* Sensor read start High */
        SIU.GPDO[SI].R = 1;

        for(i=0;i<DELAY;i++);   /* delay */
        
//      SIU.PGPDO[0].R |= 0x00000004;           /* Sensor Clock High */
        SIU.GPDO[CLK].R = 1;
        
        for(i=0;i<DELAY;i++);   /* delay */
        
//      SIU.PGPDO[0].R &= ~0x00000010;          /* Sensor read start Low */
        SIU.GPDO[SI].R = 0;
        
        for(i=0;i<DELAY;i++);   /* delay */
        
//      SIU.PGPDO[0].R &= ~0x00000004;          /* Sensor Clock Low */
        SIU.GPDO[CLK].R = 0;
        
        for(i=0;i<DELAY;i++);   /* delay */
}

void DumpCameraBuffer()
{
	for(j=0;j<128;j++)
	{
//      SIU.PGPDO[0].R |= 0x00000004;           /* Sensor Clock High */
		SIU.GPDO[CLK].R = 1;
		
		for(i=0;i<DELAY;i++);   /* delay */
		
//      SIU.PGPDO[0].R &= ~0x00000004;          /* Sensor Clock Low */
		SIU.GPDO[CLK].R = 0;
		
		for(i=0;i<DELAY;i++);   /* delay */
	}
}

void LineProcessing()
{
#ifdef THRESHOLD_DETECTION
        uint32_t sum = 0;
        float contrast;
        uint16_t min,max;
        uint16_t weight;
        int white = 0;
        
        contrast = CalculateContrast(&min,&max);
        
//        if(contrast<MINIMUM_CONTRAST)
//        	return;
        
        whiteThreshold = (uint16_t)(max - 0.40*(max-min));
        blackThreshold = (uint16_t)(min + 0.20*(max-min));
        
        //CorrectExposure();
       
        //filter result
        j = 0;
        for(i=0;i<15;i++){
        	Result[i] = whiteThreshold;
        }
        for (i = 15; i < 128-15; i++){ // ignore the first and last 16 pixels in the camera frame
       		weight = (uint16_t)fabsf(127 - (i-line_middle));
       		weight = weight*weight;
        	if (Result[i] <= blackThreshold){ //black
              	 Result[i] = blackThreshold;
				 sum += i*weight;
				 j += weight;
            }
            else if(Result[i]>=whiteThreshold){ //white
               	 Result[i] = whiteThreshold;
               	 white++;
            }
        }  
        for(i=128-15;i<128;i++){
        	Result[i] = whiteThreshold;
        }

        SendLine();
        //Check where the center of line is
        if(white>5){//j>0){
        	line_middle = (uint8_t)(line_middle + 0.95*((1.0*sum)/j-line_middle));
            CorrectExposure();
        	/**
            if(midMag != 0 && pMidMag != 0 && ppMidMag == 0){
            	int mag_avg = (midMag+pMidMag+ppMidMag)/3;
            	if(Result[line_middle] < mag_avg){
            		exposureTime -= EXPOSURE_STEP;
                	if(exposureTime<LOWER_EXPOSURE_TIME){
                		exposureTime = LOWER_EXPOSURE_TIME;
                	}
            	}
            	else if(Result[line_middle] > mag_avg){
            		exposureTime += EXPOSURE_STEP;
            		if(exposureTime>UPPER_EXPOSURE_TIME){
            			exposureTime = UPPER_EXPOSURE_TIME;
            		}
            	}
           
            }
       
            ppMidMag = pMidMag;
            pMidMag = midMag;
            midMag = Result[line_middle];
        **/
        }
        
#endif
#ifdef CONSECUTIVE_DETECTION
        uint32_t sum = 0;
        uint32_t w_sum = 0;
        float contrast;
        uint16_t min,max;
        int consect_points = 0;
        uint16_t weight;
        contrast = CalculateContrast(&min,&max);
        
//        if(contrast<MINIMUM_CONTRAST)
//        	return;
        
        whiteThreshold = (uint16_t)(max - 0.40*(max-min));
        blackThreshold = (uint16_t)(min + 0.20*(max-min));
        
        //CorrectExposure();
       
        //filter result
        j = 0;
        for(i=0;i<128;i++){
        	if(i<15 || i>113) Result[i] = whiteThreshold;
        }
        
        for (i=15; i<113; i++){ // ignore the first and last 16 pixels in the camera frame
        	if (Result[i] <= blackThreshold){ //black
        		for(j=i; Result[j]<blackThreshold && j<113; j++);
        		if((j-i)>6 ){//|| (j-i)<20){
        			for(;i<j;i++){
        				Result[i] = blackThreshold;
        	       		weight = (uint16_t)fabsf(127 - (i-line_middle));
        	       		weight = weight*weight;
        				sum += i*weight;
        				w_sum += weight;
        			}
        		}
            }
            else if(Result[i]>=whiteThreshold){ //white
               	 Result[i] = whiteThreshold;
            }
        }  

        SendLine();
        //Check where the center of line is
        if(w_sum>0){
        	line_middle = (uint8_t)(line_middle + 0.95*((1.0*sum)/w_sum-line_middle));
            //CorrectExposure();
        	/**
            if(midMag != 0 && pMidMag != 0 && ppMidMag == 0){
            	int mag_avg = (midMag+pMidMag+ppMidMag)/3;
            	if(Result[line_middle] < mag_avg){
            		exposureTime -= EXPOSURE_STEP;
                	if(exposureTime<LOWER_EXPOSURE_TIME){
                		exposureTime = LOWER_EXPOSURE_TIME;
                	}
            	}
            	else if(Result[line_middle] > mag_avg){
            		exposureTime += EXPOSURE_STEP;
            		if(exposureTime>UPPER_EXPOSURE_TIME){
            			exposureTime = UPPER_EXPOSURE_TIME;
            		}
            	}
           
            }
       
            ppMidMag = pMidMag;
            pMidMag = midMag;
            midMag = Result[line_middle];
        **/
        }
        
#endif
#ifdef WEIGHT_DETECTION
        uint8_t i = 0;
        uint32_t sum=0, weightedSum=0;
        //invert the result
        for(i=0;i<128;i++)
        {
        	Result[i]=5000-Result[i];
        }
        
        for(i=15;i<128-15;i++)
        {
        	sum += Result[i];
        	weightedSum += i*Result[i];
        }
        
        line_middle = (uint8_t)(line_middle + 0.92*((1.0*weightedSum)/(1.0*sum))-line_middle);
        //line_middle = (uint8_t)((1.0*weightedSum)/(1.0*sum));
        ppMidMag = pMidMag;
        pMidMag = midMag;
        midMag = Result[line_middle];
        
        if(midMag != 0 && pMidMag != 0 && ppMidMag == 0){
        	int mag_avg = (midMag+pMidMag+ppMidMag)/3;
        	if(Result[line_middle] < mag_avg){
        		exposureTime -= EXPOSURE_STEP;
        	}
        	else if(Result[line_middle] > mag_avg){
        		exposureTime += EXPOSURE_STEP;
        	}
        	if(exposureTime>UPPER_EXPOSURE_TIME)
        	{
        		exposureTime = UPPER_EXPOSURE_TIME;
        	}
        	else if(exposureTime<LOWER_EXPOSURE_TIME)
        	{
        		exposureTime = LOWER_EXPOSURE_TIME;
        	}
        }

#endif
#ifdef THREE_POINTS_DETECTION
        uint8_t middlePoint,rightPoint,leftPoint;
        uint16_t maxVariation=0;
        uint8_t maxMiddlePoint=0;
        
        leftPoint=0;
        middlePoint=RANGE;
        rightPoint=2*RANGE;
        
        for(i=0;i<128;i++)
        {
        	if(maxVariation<((Result[rightPoint]-Result[middlePoint])-(Result[middlePoint]-Result[leftPoint])))
        	{
        		maxVariation = (Result[rightPoint]-Result[middlePoint])-(Result[middlePoint]-Result[leftPoint]);
        		maxMiddlePoint = middlePoint;
        	}
        	
        	leftPoint++;
        	if(leftPoint>127)
        		leftPoint=127;
        	
        	middlePoint++;
        	if(middlePoint>127)
        		middlePoint=127;
        	
        	rightPoint++;
        	if(rightPoint>127)
        		rightPoint=127;
        }
        
        line_middle = (uint8_t)(line_middle + 0.92*(maxMiddlePoint-line_middle));
        
#endif
        
#ifdef THREE_POINTS_THRESHOLD_DETECTION

        uint8_t middlePoint,rightPoint,leftPoint;
        uint16_t maxVariation=0;
        uint8_t maxMiddlePoint=0;
        uint8_t byte;
        uint8_t ws,we;
        uint16_t min,max;
        float lineContrast;
        
        
        
        leftPoint=15;
        middlePoint=15+RANGE;
        rightPoint=15+2*RANGE;
        
        for(i=15;i<128-15;i++)
        {
        	if(maxVariation<((Result[rightPoint]-Result[middlePoint])-(Result[middlePoint]-Result[leftPoint])))
        	{
        		maxVariation = (Result[rightPoint]-Result[middlePoint])-(Result[middlePoint]-Result[leftPoint]);
        		maxMiddlePoint = middlePoint;
        	}
        	
        	leftPoint++;
        	if(leftPoint>127-15)
        		leftPoint=127-15;
        	
        	middlePoint++;
        	if(middlePoint>127-15)
        		middlePoint=127-15;
        	
        	rightPoint++;
        	if(rightPoint>127-15)
        		rightPoint=127-15;
        }
        
        lineContrast = CalculateContrast(&min,&max);
        byte = (uint8_t)(100*lineContrast);
        fifo_write(&out->fifo,&byte,1);
        
        if(((lineContrast>MINIMUM_CONTRAST)&&(Result[maxMiddlePoint]<=dynamicThreshold))||(faultyLine>10))
        {
        	line_middle = (uint8_t)(line_middle + 0.92*(maxMiddlePoint-line_middle));  
        	dynamicThreshold = (uint16_t)(min+0.2*(max-min));
        	faultyLine = 0;
        }
        else
        {
        	faultyLine++;
        }
#endif
        
#ifdef THREE_POINTS_WEIGHTED_DETECTION

        uint8_t middlePoint,rightPoint,leftPoint;
        uint32_t maxVariation=0;
        uint32_t newVariation=0;
        uint8_t maxMiddlePoint=0;
        uint8_t byte;
        uint8_t weight;
        float lineContrast;
        
        leftPoint=15;
        middlePoint=15+RANGE;
        rightPoint=15+2*RANGE;
        
        for(i=15;i<128-15;i++)
        {
        	if(middlePoint > line_middle)
        		weight = (uint8_t)(127 - ((int8_t)middlePoint-(int8_t)line_middle));
        	else
        		weight = 127 - ((int8_t)line_middle-(int8_t)middlePoint);
        	
        	newVariation = weight*((Result[rightPoint]-Result[middlePoint])-(Result[middlePoint]-Result[leftPoint]));
        	
        	if(maxVariation<newVariation)
        	{
        		maxVariation = newVariation;
        		maxMiddlePoint = middlePoint;
        	}
        	
        	leftPoint++;
        	if(leftPoint>127-15)
        		leftPoint=127-15;
        	
        	middlePoint++;
        	if(middlePoint>127-15)
        		middlePoint=127-15;
        	
        	rightPoint++;
        	if(rightPoint>127-15)
        		rightPoint=127-15;
        }
        
        lineContrast = CalculateContrast(0,0);
        byte = (uint8_t)(100*lineContrast);
        fifo_write(&out->fifo,&byte,1);
        
        if((lineContrast>MINIMUM_CONTRAST))
        {
        	line_middle = (uint8_t)(line_middle + 0.75*(maxMiddlePoint-line_middle));  
        }
     
#endif
        
#ifdef JEREMY_DETECTION
//        int linesFound = 0;
//        int thresshold = 100;
//        int k;
//        int left, right, middle;
//        
//        SendLine();
//        
//        for(i=14;i<128-14;i+=2){
//        	if(Result[i+2]-Result[i] < -thresshold){
//        		left = i;
//        		for(j=i+2; j<128-14; j+=2){
//        			if(Result[j+2]-Result[j] > thresshold){
//        				if(linesFound == 0){
//        					middle = j;
//        					linesFound++;
//        				}
//        				else{
//        					if(Result[j] < Result[middle])
//        					middle = j;
//        				}
//        			}
//        			for(k=j+2;k<128-14; k+=2){
//        				if(Result[j+2]-Result[j] < thresshold){
//        					right = k;
//        				}
//        			}		
//        		}
//        		i = k+2;
//        	}
//        }
        
        uint8_t middlePoint,rightPoint,leftPoint;
        uint16_t maxVariation=0;
        uint8_t maxMiddlePoint=0;
        uint8_t byte;
        int thresshold = 100;
        float lineContrast;
        
        leftPoint=15;
        middlePoint=15+RANGE;
        rightPoint=15+2*RANGE;
        
        for(i=15;i<128-15;i++)
        {
        	if(maxVariation<((Result[rightPoint]-Result[middlePoint])-(Result[middlePoint]-Result[leftPoint])))
        	{
        		maxVariation = (Result[rightPoint]-Result[middlePoint])-(Result[middlePoint]-Result[leftPoint]);
        		maxMiddlePoint = middlePoint;
        	}
        	
        	leftPoint++;
        	if(leftPoint>127-15)
        		leftPoint=127-15;
        	
        	middlePoint++;
        	if(middlePoint>127-15)
        		middlePoint=127-15;
        	
        	rightPoint++;
        	if(rightPoint>127-15)
        		rightPoint=127-15;
        }
        if(midMag == 0 || pMidMag == 0 || ppMidMag == 0){
        	ppMidMag = pMidMag;
        	pMidMag = midMag;
        	midMag = Result[maxMiddlePoint];
        	line_middle = (uint8_t)maxMiddlePoint;
        }
        else if(fabsf(Result[maxMiddlePoint] -((midMag+pMidMag+ppMidMag)/3)) <= 4*thresshold){
        	ppMidMag = pMidMag;
        	pMidMag = midMag;
        	midMag = Result[maxMiddlePoint];
        	line_middle = (uint8_t)maxMiddlePoint;
        }
#endif
        
#ifdef MINIMUM_DETECTION
        float contrast;
        uint8_t minPosition;
        int8_t ws,we;
        
        ws = (int8_t)(line_middle - SEARCH_WINDOW);
        if(ws<30)
        	ws=30;
        we = (int8_t)(line_middle + SEARCH_WINDOW);
        if(we>128-30)
        	we=128-30;
       
        //contrast = CalculateContrast(0,0);
        
        minPosition = ws;
        
        for(i=ws;i<we;i++)
        {
        	if(Result[minPosition]>Result[i])
        		minPosition = i;
        }
        
        line_middle = (uint8_t)minPosition;
//        if(midMag == 0 || pMidMag == 0 || ppMidMag == 0){
//        	ppMidMag = pMidMag;
//        	pMidMag = midMag;
//        	midMag = Result[minPosition];
//        	line_middle = (uint8_t)minPosition;
//        }
//        else if(fabsf(Result[minPosition] -((midMag+pMidMag+ppMidMag)/3)) <= 500){
//        	ppMidMag = pMidMag;
//        	pMidMag = midMag;
//        	midMag = Result[minPosition];
//        	line_middle = (uint8_t)minPosition;
//        } 
        
#endif

}

void FindBestContrast()
{
	float newContrast;
	uint16_t min, max;

	newContrast = CalculateContrast(&min,&max);
   
	if(newContrast >= actualContrast)
	{
			actualContrast = newContrast;
			baseExposureTime = exposureTime;
			dynamicThreshold = min;//(uint16_t)(min+0.1*(max-min));
	}
}

uint8_t isCameraReady()
{
	return cameraState;
}

float CalculateContrast(uint16_t* min_out,uint16_t* max_out)
{
	float newContrast;
	uint16_t max=0;
    uint16_t min=0xffff;
    int8_t ws,we;
    
    ws = (int8_t)(line_middle-SEARCH_WINDOW/2);
    if(ws<0) ws = 0;
    
    we = (int8_t)(line_middle+SEARCH_WINDOW/2);
    if(we>127) we=127;

    for(i=(uint8_t)ws; i<(uint8_t)we; i++)
    {
            max = (max > Result[i]) ? max : Result[i];
            min = (min < Result[i]) ? min : Result[i];
    }
    
    newContrast = (float)((1.0*(max - min))/(1.0*(max + min)));
    
    if(min_out!=0)
    	(*min_out)=min;
    if(max_out!=0)
    	(*max_out)=max;
    
    return newContrast;
}

void CorrectExposure()
{
	float contrast;
	uint16_t min,max;
	
	//contrast = CalculateContrast(&min,&max);
	
	if(midMag != 0 && pMidMag != 0 && ppMidMag != 0){
		
	}
	else{
		int avg = (midMag + pMidMag + ppMidMag)/3;
		if(Result[line_middle]>avg+50){
				exposureTime -= 50;
				if(exposureTime<800){
					exposureTime = LOWER_EXPOSURE_TIME;
				}
			}
			else if(Result[line_middle]<avg-50){
				exposureTime += 50;
				if(exposureTime>1800){
					exposureTime = UPPER_EXPOSURE_TIME;
				}
		}
	} 
	ppMidMag = pMidMag;
	pMidMag = midMag;
	midMag = Result[line_middle];

}

void SendRawData()
{
    uint8_t byte;
	
	//check if serial buffer is empty
    if(out->fifo.length==0)
    {
            for(i=0;i<128;i++)
            {
                    byte = lineADC[i];
                    fifo_write(&out->fifo,&byte,1);
            }
            
            byte = line_middle;
            fifo_write(&out->fifo,&byte,1);
            
            for(i=0;i<5;i++)
            {
                    byte = (i%2 == 0)?255:0;
                    fifo_write(&out->fifo,&byte,1);
            }
    }
}

void SendLine()
{
    uint8_t byte;
	
	//check if serial buffer is empty
    if(out->fifo.length==0)
    {
    	/**
    	for(i=0;i<128;i++)
        {
         	if(Result[i]==whiteThreshold)
           		byte = '.';
           	else if(Result[i]==blackThreshold)
           		byte = '|';
           	else
           		byte = 'x';
            	
            fifo_write(&out->fifo,&byte,1);
        }
        **/
/*    	uint8_t ones;
    	uint8_t tens;
    	uint8_t hunds;
    	uint8_t thous;
    	uint8_t tenThous;
    	
    	tenThous = (uint8_t)(exposureTime/10000);
    	thous = (uint8_t)((exposureTime - tenThous*10000)%1000);
    	hunds = (uint8_t)((exposureTime - tenThous*10000 - thous*1000)%100);
    	tens = (uint8_t)((exposureTime - tenThous*10000 - thous*1000 - hunds*100)%10);
    	ones = (uint8_t)(exposureTime - tenThous*10000 - thous*1000 - hunds*100 - tens*10);
    	
    	tenThous += 48; 
    	fifo_write(&out->fifo,&tenThous,1);
    	thous += 48;
    	fifo_write(&out->fifo,&thous,1);
    	hunds += 48;
    	fifo_write(&out->fifo,&hunds,1);
    	tens += 48;
    	fifo_write(&out->fifo,&tens,1);
    	ones += 48;
    	fifo_write(&out->fifo,&ones,1);
  */
    	uint32_t copyExposureTime = exposureTime;
    	for(i=0;i<5;i++)
    	{
    		byte = exposureTime%10 + 0x30;
    		exposureTime = exposureTime/10;
    		fifo_write(&out->fifo,&byte,1);
    	}
        byte = '\n';
        fifo_write(&out->fifo,&byte,1);
        byte = '\r';
        fifo_write(&out->fifo,&byte,1);
        
        exposureTime = copyExposureTime;
    }
}
