from ST7735 import TFT
from sysfont import sysfont
from machine import SPI,Pin,ADC,Timer
from rotary_irq_rp2 import RotaryIRQ

import utime

#------ LCD Init -------
spi = SPI(1, baudrate=100000000, polarity=0, phase=0, sck=Pin(10), mosi=Pin(11), miso=None)
tft=TFT(spi,8,12,9)

tft.initr()
tft.rgb(False) # board is bgr per waveshare sample
tft.invertcolor(True) # otherwise image is inverted
tft.fill(TFT.BLACK)
tft.rotation(1)

#------ ADC Init -------
VOLTAGE_TWEAK_FAKTOR=1.2535
adc_in = machine.ADC(28)

#------ Rotary Encoder init -------
rotary = RotaryIRQ( pin_num_clk=3,
                    pin_num_dt=4,
                    min_val=0,
                    max_val=2,
                    reverse=True,
                    range_mode=RotaryIRQ.RANGE_WRAP)
rotary_val = rotary.value()

rotarySel=Pin(5,Pin.IN)  

#------ GPIO -------
powerOut=Pin(22,Pin.OUT)  
powerOut.value(0)

#------ Globals -------
voltage_current = 0.00
voltage_disconnect = 12.20
state = "OFF"

timer_is_used = False
timer_min = 0
timer_seconds = 0

selector_on = False

old_voltage = 0
old_voltage_disconnect = 0
old_time = 0
old_time_s = 0
old_state = ""

lcd_selected = 0 # 0=discon, 1=timer, 2=state
lcd_change_val_mode = False


def toggle_lcd_mode(p) :
    utime.sleep_ms(1)
    if rotarySel.value()==False:
        global lcd_change_val_mode
        lcd_change_val_mode = not lcd_change_val_mode


rotarySel.irq(trigger=Pin.IRQ_FALLING, handler=toggle_lcd_mode)


#------ Timer Cyclic Interrupt -------
isr_sample_voltage_trigger = 0
isr_count_seconds_trigger = 0

def timer_ISR(p=0) :
    global isr_sample_voltage_trigger
    global isr_count_seconds_trigger
    if lcd_change_val_mode==False:
        draw_lcd_selector()
        isr_sample_voltage_trigger=isr_sample_voltage_trigger+1
        isr_count_seconds_trigger=isr_count_seconds_trigger+1
        
        if isr_sample_voltage_trigger == 20:
            sample_voltage()
            drawScreen_Voltage()
            isr_sample_voltage_trigger = 0

    if isr_count_seconds_trigger == 10:
        handle_time_every_second()
        drawScreen_Timer()
        isr_count_seconds_trigger = 0        


        
timer = Timer(period=100,mode=Timer.PERIODIC,callback=timer_ISR)



#------ Functions -------

def drawScreen_State() :
    global old_state
    if(old_state!=state):
        tft.text((85, 80), old_state, TFT.BLACK, sysfont, 2)
        tft.text((85, 80), state, TFT.WHITE, sysfont, 2)
        old_state = state

def drawScreen_Voltage() :
    global old_voltage
    if(int(old_voltage*100)!=int(voltage_current*100)):
        tft.text((15, 40), '{0:.2f}'.format(old_voltage), TFT.BLACK, sysfont, 2)
        tft.text((15, 40), '{0:.2f}'.format(voltage_current), TFT.WHITE, sysfont, 2)
        old_voltage = voltage_current

def drawScreen_Disconnect() :
    global old_voltage_disconnect
    if(old_voltage_disconnect!=voltage_disconnect):
        tft.text((85, 40), '{0:.2f}'.format(old_voltage_disconnect), TFT.BLACK, sysfont, 2)
        tft.text((85, 40), '{0:.2f}'.format(voltage_disconnect), TFT.WHITE, sysfont, 2)
        old_voltage_disconnect = voltage_disconnect

def drawScreen_Timer() :
    global old_time
    global old_time_s
    if old_time!=timer_min:
        tft.text((15, 80), str(old_time), TFT.BLACK, sysfont, 2)
        tft.text((15, 80), str(timer_min), TFT.WHITE, sysfont, 2)
        old_time = timer_min

    if old_time_s!=timer_seconds:
        tft.text((50, 80), str(old_time_s), TFT.BLACK, sysfont, 1)
        tft.text((50, 80), str(timer_seconds), TFT.WHITE, sysfont, 1)
        old_time_s = timer_seconds

def drawScreen():
    drawScreen_State()
    drawScreen_Voltage()
    drawScreen_Disconnect()
    drawScreen_Timer()

def draw_lcd_selector() :
    global lcd_change_val_mode
    global voltage_disconnect
    global selector_on    
    global timer    
    selector_on = not selector_on

    if lcd_selected == 0: # disconnect
        highlight_timer_off()
        highlight_state_off()
        if selector_on==True:
            highlight_disconnect() 
        else:
            highlight_disconnect_off()
    elif lcd_selected == 1: # timer
        highlight_disconnect_off()
        highlight_state_off()
        if selector_on==True:
            highlight_timer() 
        else:
            highlight_timer_off()
    elif lcd_selected == 2: # state
        highlight_timer_off()
        highlight_disconnect_off()
        if selector_on==True:
            highlight_state() 
        else:
            highlight_state_off()

def highlight_state():
    tft.line((85, 100), (145,100), tft.RED)
def highlight_state_off():
    tft.line((85, 100), (145,100), tft.BLACK)

def highlight_disconnect():
    tft.line((85, 60), (145,60), tft.RED)
def highlight_disconnect_off():
    tft.line((85, 60), (145,60), tft.BLACK)

def highlight_timer():
    tft.line((15, 100), (75,100), tft.RED)
def highlight_timer_off():
    tft.line((15, 100), (75,100), tft.BLACK)

def set_state_value():
    global state
    highlight_state()
    highlight_disconnect_off()
    highlight_timer_off()
    rotary.set(value=0,min_val=0,max_val=1)
    while lcd_change_val_mode ==True:
        if rotary.value()==0:
            state="OFF"
        else:
            state="ON"
        drawScreen_State()

def set_timer_value():
    global timer_min
    global timer_seconds
    global timer_is_used
    highlight_state_off()
    highlight_disconnect_off()
    highlight_timer()
    rotary.set(value=timer_min,min_val=0,max_val=1440)
    while lcd_change_val_mode ==True:
        timer_min = rotary.value()
        drawScreen_Timer()
    if timer_min != 0:
        timer_is_used = True    


def set_disconnect_value():
    global voltage_disconnect
    highlight_state_off()
    highlight_disconnect()
    highlight_timer_off()
    rotary.set(value=round(voltage_disconnect*100),min_val=1190,max_val=1300)
    while lcd_change_val_mode ==True:
        voltage_disconnect = (rotary.value())/100
        drawScreen_Disconnect()

def change_lcd_val():
    global lcd_change_val_mode
    global voltage_disconnect
    if lcd_selected == 0: # disconnect
        set_disconnect_value()            
    elif lcd_selected == 1: # timer
        set_timer_value()
    elif lcd_selected == 2: # state
        set_state_value()
    rotary.set(value=0,min_val=0,max_val=2)


def handle_time_every_second():
    global timer_seconds
    global timer_min
    global timer_is_used
    if timer_is_used == True and timer_min > 0 and timer_seconds == 0:
        timer_seconds = 60
        timer_min = timer_min - 1

    if timer_is_used == True and timer_seconds > 0:
        timer_seconds = timer_seconds - 1

        

def sample_voltage():
    adc_sum=0
    global voltage_current
    for i in range(100):
        adc_sum = adc_sum+adc_in.read_u16()
    adc_uint16 = adc_sum/100
    voltage_current = (18*adc_uint16*VOLTAGE_TWEAK_FAKTOR)/(65535)

def eval_power_output():
    global state
    if (   
       float(voltage_current) >= float(voltage_disconnect) and
       (timer_is_used == False or timer_min > 0 or timer_seconds > 0) and
       state=="ON"
    ):
        powerOut.value(1)
    elif state=="ON":
        state="OFF"
        drawScreen_State()
        powerOut.value(0)
    else:
        powerOut.value(0)

#------ Draw Static Display -------
tft.text((15, 30), "Current", TFT.WHITE, sysfont, 1)
tft.text((85, 30), "Disconnect", TFT.WHITE, sysfont, 1)
tft.text((15, 65), "Timer", TFT.WHITE, sysfont, 1)
tft.text((85, 65), "State", TFT.WHITE, sysfont, 1)
drawScreen()
sample_voltage()
drawScreen_Voltage()
#------ Endlessloop: -------

while True:

    # Rotary Encoder was pressed to change a value:
    if lcd_change_val_mode == True:
        change_lcd_val()

    # Normal LCD update
    else:
        if lcd_selected != rotary.value():
            lcd_selected = rotary.value()
            draw_lcd_selector()

    eval_power_output()