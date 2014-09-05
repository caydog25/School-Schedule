// Raymond Huffman
//
// 9/5/2014

#include <pebble.h>

static Window *mainwindow;
static TextLayer *xtime, *xdate, *xperiod, *xmeridian;  

static int STARTHOUR = 8;
static int STARTMIN = 18;
static int DURATION = 42;
static int BREAKDUR = 4;

struct course
{
  int period;
  int starthour;
  int startmin;
  int endhour;
  int endmin;
};

struct course courses[9];

static void createCourses()
{
  for(int x = 0; x < 9; x++)
  {
    courses[x].period = x+1;
    if(x==0)
    {
      courses[x].starthour = STARTHOUR;
      courses[x].startmin = STARTMIN;
      courses[x].endhour = courses[x].starthour;
      courses[x].endmin = courses[x].startmin + DURATION;
      
      if (courses[x].endmin>=60)
      {
        courses[x].endhour = courses[x].endhour+1;
        courses[x].endmin = courses[x].endmin - 60;
      }
    }
    else
    {
      courses[x].startmin = courses[x-1].startmin + DURATION + BREAKDUR;
      if (courses[x].startmin>=60)
      {
        courses[x].starthour = courses[x-1].starthour+1;
        courses[x].startmin = courses[x].startmin - 60;
      }
      else
      {
        courses[x].starthour = courses[x-1].starthour;  
      }
   
    
      courses[x].endmin = courses[x].startmin + DURATION;
      if (courses[x].endmin>=60)
      {
        courses[x].endhour = courses[x-1].endhour+1;
        courses[x].endmin = courses[x].endmin - 60;
      }
      else
      {
        courses[x].endhour = courses[x-1].endhour;  
      }
    }
  }
  
  for(int x = 0; x < 9; x++)
  {
    static char starttimelogbuffer[] = "00:00";
    snprintf(starttimelogbuffer, sizeof(starttimelogbuffer), "%d:%d", courses[x].starthour, courses[x].startmin);
    APP_LOG(APP_LOG_LEVEL_INFO, starttimelogbuffer);
    
    static char endtimelogbuffer[] = "00:00";
    snprintf(endtimelogbuffer, sizeof(endtimelogbuffer), "%d:%d", courses[x].endhour, courses[x].endmin);
    APP_LOG(APP_LOG_LEVEL_INFO, endtimelogbuffer);
    
  }
}


  
  
static void update_time()
{
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
    
  static char timebuffer[] = "00:00";
  static char datebuffer[] = "dayofweek mm/dd/yyyy";  
  static char meridianbuffer[] = "AM";
  
  strftime(timebuffer, sizeof(timebuffer), "%I:%M", tick_time);
  strftime(datebuffer, sizeof(datebuffer), "%A\n%m/%d/%Y", tick_time);
  strftime(meridianbuffer, sizeof(meridianbuffer), "%p", tick_time);
  
  if(timebuffer[0]=='0')
    timebuffer[0]=' ';
  
  text_layer_set_text(xtime, timebuffer);
  text_layer_set_text(xdate, datebuffer);
  text_layer_set_text(xmeridian, meridianbuffer);
  
  int currentperiod = 1;
  int currenttimemins = tick_time->tm_hour * 60 + tick_time->tm_min;
  bool inclass = false;
  bool insession = false;
  
  for(int x = 0; x<9; x++)
  {
    if((currenttimemins>=courses[x].starthour * 60 + courses[x].startmin) && (currenttimemins<courses[x].endhour * 60 + courses[x].endmin))
    {
           currentperiod=x+1;
           inclass=true;
           insession=true;
    }
    if(x!=0) 
    {
      if((currenttimemins<courses[x].starthour * 60 + courses[x].startmin) && (currenttimemins>=courses[x-1].endhour * 60 + courses[x-1].endmin))
      {
           currentperiod=x+1;
           inclass=false;
           insession=true;
        
      } 
    }   
  }
  
  int current_minute_buffer;
  int schedule_minute_buffer;
  
  int mins_left = 0;
  int hours_left = 0;
  
  static char verb[] = "starts";
  
  if(inclass)
  {
    current_minute_buffer = currenttimemins;
    schedule_minute_buffer = courses[currentperiod-1].endhour * 60 + courses[currentperiod-1].endmin;
    strcpy(verb, "ends");
  }
  else
  {
    current_minute_buffer = currenttimemins;
    schedule_minute_buffer = courses[currentperiod-1].starthour * 60 + courses[currentperiod-1].startmin;
    strcpy(verb, "starts");
  }
  
  if(currenttimemins<=courses[0].starthour * 60 + courses[0].startmin)
  {
    insession=true;
    current_minute_buffer = currenttimemins;
    schedule_minute_buffer = courses[0].starthour * 60 + courses[0].startmin;
  }
  
  mins_left = schedule_minute_buffer-current_minute_buffer;
  
  if(mins_left>=60)
    {
      hours_left = mins_left/60;
      mins_left = mins_left - hours_left*60;
    }
  static char schedulebuffer[] = "Period X starts in: 00:00";
  if (insession)
  {
    snprintf(schedulebuffer, sizeof(schedulebuffer), "Period %d %s in: %2d:%02d", currentperiod, verb, hours_left, mins_left);
    text_layer_set_text(xperiod, schedulebuffer);
  }
  else
  {
    snprintf(schedulebuffer, sizeof(schedulebuffer), "%s", "No classes");
    text_layer_set_text(xperiod, schedulebuffer);
  }
  
  
  
}



static void mainwindowLoad(Window *pointer)
{
  xtime = text_layer_create(GRect(0, 0, 144, 50));
  xdate = text_layer_create(GRect(0, 105, 144, 65));
  xperiod = text_layer_create(GRect(0, 50, 144, 60));
  xmeridian = text_layer_create(GRect(118, 34, 24, 24));
  
  window_set_background_color(pointer, GColorBlack);
  text_layer_set_background_color(xtime, GColorBlack);
  text_layer_set_background_color(xdate, GColorBlack);
  text_layer_set_background_color(xperiod, GColorBlack);
  text_layer_set_background_color(xmeridian, GColorBlack);
  
  text_layer_set_text_color(xtime, GColorWhite);
  text_layer_set_text_color(xdate, GColorWhite);
  text_layer_set_text_color(xperiod, GColorWhite);
  text_layer_set_text_color(xmeridian, GColorWhite);
  
  text_layer_set_text(xtime, "12:34");
  text_layer_set_text(xdate, "8 - 1 - 2014");
  text_layer_set_text(xperiod, "No classes");
  text_layer_set_text(xmeridian, "AM");
  
  text_layer_set_font(xtime, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_font(xdate, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_font(xperiod, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_font(xmeridian, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    
  text_layer_set_text_alignment(xtime, GTextAlignmentCenter);
  text_layer_set_text_alignment(xdate, GTextAlignmentCenter);
  text_layer_set_text_alignment(xperiod, GTextAlignmentCenter);
  text_layer_set_text_alignment(xmeridian, GTextAlignmentCenter);
  
  text_layer_set_overflow_mode(xdate, GTextOverflowModeWordWrap);
  text_layer_set_overflow_mode(xperiod, GTextOverflowModeWordWrap);
  
  update_time();
  
  layer_add_child(window_get_root_layer(pointer), text_layer_get_layer(xtime));
  layer_add_child(window_get_root_layer(pointer), text_layer_get_layer(xdate));
  layer_add_child(window_get_root_layer(pointer), text_layer_get_layer(xperiod));
  layer_add_child(window_get_root_layer(pointer), text_layer_get_layer(xmeridian));
}

static void mainwindowUnload(Window *pointer)
{
  text_layer_destroy(xtime);
  text_layer_destroy(xdate);
  text_layer_destroy(xperiod);
  text_layer_destroy(xmeridian);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();
}

static void init()
{
  mainwindow = window_create();
 
  createCourses();
  
  WindowHandlers handlers;
    handlers.load = mainwindowLoad;
    handlers.unload = mainwindowUnload;
    handlers.appear = NULL;
    handlers.disappear = NULL;
  window_set_window_handlers(mainwindow, handlers);
  
  
  
  window_stack_push(mainwindow, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit()
{
  window_destroy(mainwindow);
}


int main(void)
{
  init();
  app_event_loop();
  deinit();
}