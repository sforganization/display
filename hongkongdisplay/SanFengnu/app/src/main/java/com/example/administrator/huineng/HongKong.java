package com.example.administrator.huineng;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.graphics.Color;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.example.administrator.huineng.R;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Timer;
import java.util.TimerTask;

public class HongKong extends AppCompatActivity {

    private  View glassInclude[] = new View[3];

    private  Button glassOpenLock[] = new Button[3];
    private  Button glassTrunOn[] = new Button[3];
    private  Button glassBottomCicle[] = new Button[3];
    private  Button glassTopCicle[] = new Button[3];
    private ProgressBar process_bar[] = new  ProgressBar[3];


    private  View hongKong3_include = null;
    private  Button hongKong3_includeCicle = null;


    Button hongKong1_run = null;
    Button hongKong2_up = null;
    Button hongKong2_down = null;
    Button hongKong2_lock = null;

    protected  int dir_mode[] = new int[3];
    protected  int tpd_mode[] = new int[3];

    protected  int hongKong1_run_state;
    protected  int hongKong2_run_state;

    protected  int pressState[] = new int[3];   //

    protected  volatile int lock_timer[] = new int[3];   //开锁时间，最长，没有收到数据就默认已经关锁
    protected  volatile int trun_timer[] = new int[3];   //转动时间，最长，没有收到数据就默认已经关锁


    protected void hideBottomUIMenu() {
        if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) {
            View v = this.getWindow().getDecorView();
            v.setSystemUiVisibility(View.GONE);
        } else if (Build.VERSION.SDK_INT >= 19) {
            Window window = getWindow();
            int uiOptions =View.SYSTEM_UI_FLAG_LOW_PROFILE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
                    | View.SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
                    | View.SYSTEM_UI_FLAG_IMMERSIVE;
            window.getDecorView().setSystemUiVisibility(uiOptions);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                window.setStatusBarColor(Color.TRANSPARENT);
            }
            window.addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
        }

        Window _window;
        _window = getWindow();

        WindowManager.LayoutParams params = _window.getAttributes();
        params.systemUiVisibility = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
        _window.setAttributes(params);
    }

    public static   String g_passwd = new String("");

    private com.example.x6.serial.SerialPort serialttyS0;  //使用的是S2
    private InputStream ttyS0InputStream;//使用的是S2
    private OutputStream ttyS0OutputStream;//使用的是S2
    /* 打开串口 */
    private void init_serial() {
        try {
            //使用的是S2

            serialttyS0 = new com.example.x6.serial.SerialPort(new File("/dev/ttyS2"),115200,0, 10);
//            serialttyS0 = new com.example.x6.serial.SerialPort(new File("/dev/ttyS2"),115200,0, 10);
            ttyS0InputStream = serialttyS0.getInputStream();
            ttyS0OutputStream = serialttyS0.getOutputStream();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /*
    //    包头 + 命令 +参数[3] +数据[3]    +  校验和 + 包尾
    //    包头：0XAA
    //    命令： CMD_UPDATE   、 CMD_READY
    //    参数： 地址，LED模式，增加删除指纹ID,
    //    数据：（锁开关 + 方向 + 时间）
    //    校验和：
    //    包尾： 0X55
    //    参数传递过来是命令+参数+数据  data[7]
     */
    public byte[] makeStringtoFramePackage(byte[] data)
    {
        //在时间byte[]前后添加一些package校验信息
        int dataLength   = 8;
        byte CheckSum     = 0;
        byte[] terimalPackage=new byte[10];

        //装填信息
        //时间数据包之前的信息
        terimalPackage[0] = (byte)0xAA;			   //包头
        terimalPackage[1] = data[0];         //cmd 命令
        terimalPackage[2] = data[1];         //参数0  地址，LED模式，增加删除指纹ID,
        terimalPackage[3] = data[2];         //
        terimalPackage[4] = data[3];         //
        terimalPackage[5] = data[4];         //数据  （锁开关 + 方向 + 时间）
        terimalPackage[6] = data[5];         //
        terimalPackage[7] = data[6];         //

        //计算校验和
        //转化为无符号进行校验
        for(int dataIndex = 0; dataIndex < 8; dataIndex++)
        {
            CheckSum += terimalPackage[dataIndex];
        }
        terimalPackage[8] = (byte)(~CheckSum);          //检验和
        terimalPackage[9] = (byte)0x55;            //包尾
        return terimalPackage;
    }

    private void sendFingerENPack() {  //发送包
//        terimalPackage[0] = (byte)0xAA;			   //包头
//        terimalPackage[1] = (byte)data[0];         //cmd 命令
//        terimalPackage[2] = (byte)data[1];         //参数0  地址，LED模式，增加删除指纹ID,
//        terimalPackage[3] = (byte)data[2];         //
//        terimalPackage[4] = (byte)data[3];         //
//        terimalPackage[5] = (byte)data[4];         //数据  （锁开关 + 方向 + 时间）
//        terimalPackage[6] = (byte)data[5];         //
//        terimalPackage[7] = (byte)data[6];         //
        byte[] temp_bytes = new byte[]{0x09, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};  // 09 使能指纹接收命令，0x11保留参数
        byte[] send = makeStringtoFramePackage(temp_bytes);
        /*串口发送字节*/
        try {
            ttyS0OutputStream.write(send);
            //ttyS1InputStream.read(send);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void sendFingerDisablePack() {  //发送包
        byte[] temp_bytes = new byte[]{0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};  // 0A 不使能指纹接收命令，0x11保留参数
        byte[] send = makeStringtoFramePackage(temp_bytes);
        /*串口发送字节*/
        try {
            ttyS0OutputStream.write(send);
            //ttyS1InputStream.read(send);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    private void sendReadyPack() {  //发送就绪包
        byte[] temp_bytes = new byte[]{0x03, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};  // 03 reday 命令，0x11保留参数
        byte[] send = makeStringtoFramePackage(temp_bytes);
        /*串口发送字节*/
        try {
            ttyS0OutputStream.write(send);
            //ttyS1InputStream.read(send);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    //    包头 + 地址 + 命令 + 数据包[48] + 校验和 + 包尾
    //    包头：0XAA
    //    手表地址：0X8~：最高位为1表示全部                       ~:表示有几个手表
    //              0x01：第一个手表
    //    命令：
    //    数据：（锁开关 + 方向 + 时间）* 16 = 48
    //    校验和：
    //    包尾： 0X55

    private byte[] recvData = new byte[128];    //每次最大是32位
    private byte[] recvArry = new byte[256];    //设置缓存大概4级    256/ 53 ~ 4
    private int i= 0;
    private int index= 0;
    private int cnt= 0;
    private int sizeRec= 0;
    private byte[] glassData = new byte[50];    //一个完整数据包
    private byte checkSum = 0;
    private byte ackCheck= (byte)0xFF;  //一个字节的应答判断
    private byte zero_num= (byte)0;  //记录第几个手表进入到零点


    protected int jungleRecPack() {   //

        for(index = 0; index < 255; index++)
        {
            if(recvArry[index] == (byte)0xAA)  //如果是包头
            {
                break;
            }
        }

        if(index == 255) //出错
            return -1;
        checkSum = 0;
        for(i = 0 ; i < 53; i++) //和校验
        {
            checkSum += recvArry[index + i];
        }

        if(checkSum != (byte)0x55 - 1)
            return -1; //

        //应答地址 0xFF
//        u8aSendArr[1] = MCU_ACK_ADDR;//应答地址
//        u8aSendArr[2] = ZERO_ACK_TYPE;       //应答类型零点
//        u8aSendArr[3] = u8GlassAddr;       //手表地址
        if(recvArry[index + 1] == (byte)0xff)        {
            if(recvArry[index + 2]  == (byte)0x02)   //0x01 零点应答类型
            {
                zero_num = recvArry[index + 3];
                ackCheck = 0;   //校验正确

                return 0;
            }
        }

        ackCheck = (byte) 0xFF;
        return 0;
    }

    volatile  private boolean g_exit = false;

    protected int checkRecPack() {   //串口接收数据

        int tmp_cnt = 0;

        try {
            while(g_exit != true) {
                sizeRec = 0;
                do {
                    cnt = -1;
                    if(ttyS0InputStream != null)
                        cnt = ttyS0InputStream.read(recvData);
                    if (cnt != -1) {
                        System.arraycopy(recvData, 0, recvArry, sizeRec, cnt);
                        sizeRec += cnt;
                    }
                } while ((sizeRec < 53) && (g_exit == false));  //少于一个包数据  //或者已经退出

                if (g_exit == true)
                    return -1;

                if (jungleRecPack() == 0) //检查参数合法性
                {
                    //if(zero_num ==  glass_num)
                    break;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return 0;
    }



    MyThread myThread = new MyThread();  //串口接收数据

    public class MyThread extends Thread {
        private boolean suspend = false;

        private String control = ""; // 只是需要一个对象而已，这个对象没有实际意义

        public void setSuspend(boolean suspend) {
            if (!suspend) {
                synchronized (control) {
                    control.notifyAll();
                }
            }
            this.suspend = suspend;
        }

        public boolean isSuspend() {
            return this.suspend;
        }

        public void run() {
            while (true) {
                synchronized (control) {
                    if (suspend) {
                        try {
                            control.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }

                while(!isInterrupted() && (suspend == false)) {
                    if(checkRecPack() == 0) {
                        //由timer发送消息
                        //要判断是哪个锁到了零点
                        if(lock_timer[zero_num] > 0)
                            lock_timer[zero_num] = 2;  //2ms后发送
                        // if(trun_timer > 0) trun_timer = 2;
                    }
                }
            }
        }

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        int i;

        g_exit =  true;
        if (myThread != null) {
            Log.d("aaa", "honkong 1  发送线程停止interrupt。。。。。。");
            myThread.setSuspend(true);
            myThread.interrupt();
        }

        tick_task.cancel();
        serial_task.cancel();
        // 移除所有消息
        handler.removeCallbacksAndMessages(null);
        // 或者移除单条消息
        for(i = 0; i < 16; i++) {
            handler.removeMessages((0x80 | i));
            handler.removeMessages((0x40 | i));
        }

        //关闭串口
        try {
            ttyS0InputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {
            ttyS0OutputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (serialttyS0 != null) {
            serialttyS0.close();
            serialttyS0 = null;
        }
    }

    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler(){

        int msgNum = 0;
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            msgNum = msg.what;

            if(msgNum == 0x1100){
                //g_exit = true;   //只暂停， 停止线程会出现 问题

                stopTimer();
                handler.removeCallbacksAndMessages(null);
                Intent intent = new Intent();
                intent.setClass(HongKong.this, Advertisment.class);
                startActivityForResult(intent, 1);
            }

            else if((msgNum & (byte)(0x80)) != 0) {
                msgNum &= (byte)(0x7F);
                glassTrunOn[msgNum].setEnabled(true);
                glassOpenLock[msgNum].setVisibility(View.INVISIBLE);
                glassTrunOn[msgNum].setVisibility(View.VISIBLE);
                glassTopCicle[msgNum].setVisibility(View.INVISIBLE);
                glassBottomCicle[msgNum].setVisibility(View.VISIBLE);
                process_bar[msgNum].setVisibility(View.GONE);
            }else {
                msgNum &= (byte)(0x3F);
                glassOpenLock[msgNum].setEnabled(true);
                glassOpenLock[msgNum].setVisibility(View.VISIBLE);
                glassTrunOn[msgNum].setVisibility(View.INVISIBLE);
                glassTopCicle[msgNum].setVisibility(View.VISIBLE);
                glassBottomCicle[msgNum].setVisibility(View.INVISIBLE);
                process_bar[msgNum].setVisibility(View.GONE);
            }
        }
    };

    public void changeViewShade() {

        hongKong1_run.setBackgroundResource(R.drawable.hong_kong_button_shade);
        hongKong2_up.setBackgroundResource(R.drawable.hong_kong_button_shade);
        hongKong2_down.setBackgroundResource(R.drawable.hong_kong_button_shade);
//hong_kong_button
        if(hongKong1_run_state == 0) //转动状态
        {
            hongKong1_run.setBackgroundResource(R.drawable.hong_kong_button);
        }

        if(hongKong2_run_state == 1) //下降
        {
            hongKong2_up.setBackgroundResource(R.drawable.hong_kong_button);
        }else{//上升
            hongKong2_down.setBackgroundResource(R.drawable.hong_kong_button);
        }
    }

    volatile int movieTimes = 0;

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == 1){
            movieTimes = 0;
            myThread.setSuspend(false);
            isReady = true;
            startTimer();
        }
    }


    Timer timer = null;
    TimerTask tick_task= null;
    TimerTask serial_task= null;
    volatile boolean isReady = true;

    private void startTimer(){
        if (timer == null) {
            timer = new Timer();
        }

        if (serial_task == null) {
            serial_task = new TimerTask() {
                @Override
                public void run() {
                    /**
                     *要执行的操作
                     */

                    myThread.start();        /*延时一段时间再开启线程*/
                }
            };
        }
        else {
            myThread.setSuspend(false);
        }

        if (tick_task == null) {
            tick_task = new TimerTask() {
                @Override
                public void run() {

                            if(isReady == false)
                                return;

                            /**
                             *要执行的操作
                             */
                            movieTimes++;
                            if(movieTimes >= OPEN_LOCK_TIME + 300)//屏幕没有操作跳到广告页面
                            {
                                movieTimes = 0;
                                isReady = false;
                                Message msg = Message.obtain();
                                handler.sendEmptyMessageDelayed((0x1100), 1);  //发送消息
                            }

                            for(i = 0; i < 3; i++){
                                if(lock_timer[i] > 0){
                                    lock_timer[i]--;
                                    if(lock_timer[i] == 0)
                                    {
                                        Message msg = Message.obtain();
                                        handler.sendEmptyMessageDelayed((i | 0x80), 1);  //单片机收到零点，发送消息停止process
                                    }
                                }

                                if(trun_timer[i] > 0){
                                    trun_timer[i]--;
                                    if(trun_timer[i] == 0)
                                    {
                                        Message msg = Message.obtain();
                                        handler.sendEmptyMessageDelayed((i | 0x40), 1);  //发送消息
                                    }
                                }
                            }
                }
            };
        }

        if(timer != null)
        {
            if(tick_task != null ) {
                timer.schedule(tick_task, 10, 1);//1ms后执行Tick   1ms 的tick
            }

        }
    }

    private void stopTimer(){

        isReady = false;
        if (serial_task != null) {
            myThread.setSuspend(true);
          /*  myThread.exit = true;
            try {
                myThread.sleep(200);
                myThread.interrupt();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            serial_task.cancel();
            serial_task = null;*/
        }

        if (tick_task != null) {
            tick_task.cancel();
            tick_task = null;
        }

        if (timer != null) {
            timer.cancel();
            timer = null;
        }
        System.gc();
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        hideBottomUIMenu();


        Intent intent = getIntent();
        Bundle myBudle = this.getIntent().getExtras();
        glassData = myBudle.getByteArray("glassData");

        for(i = 0; i < 3; i++){
            dir_mode[i] = glassData[3 + 3 * i];
            tpd_mode[i] = glassData[3 + 3 * i + 1];
        }

        if(dir_mode[0] == 0x02) //正反转
            hongKong1_run_state = 0;
        else
            hongKong1_run_state = 1; //0x06 停止

        hongKong2_run_state = glassData[6];   //0 正转，上升  1 反转 下降

        setContentView(R.layout.activity_hong_kong);

        for(i = 0; i < 3; i++)
        {
            pressState[i] = 1; //默认赋值关锁状态
        }

        initViews();
        init_serial();          //初始化串口
        startTimer();
        changeViewShade();
        if(serial_task != null ) {
            timer.schedule(serial_task, 50);//300ms后执行TimeTask的run方法
        }
    }

    private void initViews() {
        //这是获得include中的控件
        glassInclude[0] = findViewById(R.id.hongkong1_lock);
        glassInclude[2] = findViewById(R.id.hongkong3_lock);
        hongKong3_include = findViewById(R.id.Hongkong3_mode);

        //获取include中的子控件

        hongKong3_includeCicle = hongKong3_include.findViewById(R.id.include_cicle);
        glassOpenLock[0] = glassInclude[0] .findViewById(R.id.include_open_lock_button);
        glassOpenLock[2] = glassInclude[2] .findViewById(R.id.include_open_lock_button);

        glassTrunOn[0]  = glassInclude[0] .findViewById(R.id.include_trun_moto_button);
        glassTrunOn[2]  = glassInclude[2] .findViewById(R.id.include_trun_moto_button);

        glassTopCicle[0]  = glassInclude[0] .findViewById(R.id.include_top_circle);
        glassTopCicle[2]  = glassInclude[2] .findViewById(R.id.include_top_circle);

        glassBottomCicle[0]  = glassInclude[0] .findViewById(R.id.include_bottom_circle);
        glassBottomCicle[2]  = glassInclude[2] .findViewById(R.id.include_bottom_circle);

        process_bar[0] = findViewById(R.id.hongkong1_process);
        process_bar[2] = findViewById(R.id.hongkong3_process);

        hongKong1_run = findViewById(R.id.Hongkong1_trun);
        hongKong2_up = findViewById(R.id.Hongkong2_up);
        hongKong2_down = findViewById(R.id.Hongkong2_down);
        hongKong2_lock = findViewById(R.id.Hongkong2_lock);

        // 3.设置按钮点击事件
        glassOpenLock[0] .setOnClickListener(onClickListener);
        glassOpenLock[2] .setOnClickListener(onClickListener);

        glassTrunOn[0] .setOnClickListener(onClickListener);
        glassTrunOn[2] .setOnClickListener(onClickListener);
        hongKong3_includeCicle.setOnClickListener(onClickListener);
        hongKong1_run.setOnClickListener(onClickListener);
        hongKong2_up.setOnClickListener(onClickListener);
        hongKong2_down.setOnClickListener(onClickListener);
        hongKong2_lock.setOnClickListener(onClickListener);
    }


    final int TRUN_TIME = 1000;
    final int OPEN_LOCK_TIME = 6000;
    // 2.得到 OnClickListener 对象
    View.OnClickListener onClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            byte[] temp_bytes = new byte[]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // 0x02 更新状状态命令
            boolean inputCheck = true; //默认有效
            boolean inputCheck2 = true; //默认有效
            boolean openlock = false;
            byte keyNum = 0; //按键号


            movieTimes = 0;  //清空这个视频时间，有按下重新开始计时
            //if (v.getId() == R.id.lock_16_return) {    //把返回键
            switch (v.getId()) {
                case R.id.Hongkong1_trun:
                    keyNum = 0;
                    hongKong1_run_state ^= 1;
                    if(hongKong1_run_state == 0) //正常，正反转
                        temp_bytes[5] = (byte) 0x02;  //方向 MOTO_FR_FWD_REV 0x02
                    else //停止
                        temp_bytes[5] = (byte) 0x03;  //方向 MOTO_FR_STOP 0x03

                    //temp_bytes[6] = (byte)tpd_mode[0]; //时间 mode MOTO_TIME_HALF 0x06
                    temp_bytes[6] = (byte)0x06; //时间 mode MOTO_TIME_HALF 0x06
                    break;

                case R.id.Hongkong2_up:
                    keyNum = 1;
                    if(lock_timer[keyNum] != 0){
                        Toast toast= Toast.makeText(HongKong.this,"正在开锁。。。!",Toast.LENGTH_SHORT);
                        toast.show();
                        return;
                    }

                    hongKong2_run_state = 0;
                    temp_bytes[5] = (byte) hongKong2_run_state;  //方向
                    temp_bytes[6] = (byte)tpd_mode[1]; //时间
                    hongKong2_up.setEnabled(false); //禁止按下
                    hongKong2_down.setEnabled(true); //可以按下
                    break;

                case R.id.Hongkong2_down:
                    keyNum = 1;
                    if(lock_timer[keyNum] != 0){
                        Toast toast= Toast.makeText(HongKong.this,"正在开锁。。。!",Toast.LENGTH_SHORT);
                        toast.show();
                        return;
                    }

                    hongKong2_run_state = 1;
                    temp_bytes[5] = (byte) hongKong2_run_state;  //方向
                    temp_bytes[6] = (byte)tpd_mode[1]; //时间
                    hongKong2_up.setEnabled(true); //可以按下
                    hongKong2_down.setEnabled(false); //禁止按下
                    break;
                case R.id.Hongkong2_lock:
                    keyNum = 1;
                    if(hongKong2_run_state != 0){
                        Toast toast= Toast.makeText(HongKong.this,"请先升起。。。",Toast.LENGTH_SHORT);
                        toast.show();
                        return;
                    }

                    openlock = true;
                    break;

                default:
                    inputCheck2 = false;
                    break;
            }

            if(inputCheck2) {
                changeViewShade();
                if(openlock == true) {
                    temp_bytes[0] = (byte) 0x06;       //0x06 开锁命令
                    temp_bytes[4] = (byte) 0;//锁开关
                }else{
                    temp_bytes[0] = (byte) 0x02;       //0x02 更新状状态命令
                }

                temp_bytes[1] = keyNum; //地址 参数0  地址，LED模式，增加删除指纹ID,

                //temp_bytes[5] = (byte)dir_mode;  //方向
                //temp_bytes[6] = (byte) 0;  //时间

                byte[] send = makeStringtoFramePackage(temp_bytes);
                /*串口发送字节*/
                try {
                    ttyS0OutputStream.write(send);
                    //ttyS1InputStream.read(send);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            /*******************开锁按键处理**************************/
            //先取入include 布件
            //获得父控件的对象，然后获得父控件的id
            ViewGroup parent = (ViewGroup) v.getParent();
            switch (parent.getId()) {
                case R.id.hongkong1_lock:
                    keyNum = 0;
                    if (v.getId() == R.id.include_open_lock_button) {
                        if (pressState[keyNum] == 1) {
                            pressState[keyNum] = 0;//1表示开锁已经按下    parent.include_open_lock_button.setEnabled(false); //禁止按下
                            lock_timer[keyNum] = OPEN_LOCK_TIME;
                        }
                    } else {
                        if (pressState[keyNum] == 0) {
                            pressState[keyNum] = 1;  //1表示转动已经按下
                            trun_timer[keyNum] = TRUN_TIME;
                        }
                    }
                    break;

                case R.id.hongkong3_lock:
                    keyNum = 2;
                    if (v.getId() == R.id.include_open_lock_button) {
                        if (pressState[keyNum] == 1) {
                            pressState[keyNum] = 0;//1表示开锁已经按下                        parent.include_open_lock_button.setEnabled(false); //禁止按下
                            lock_timer[keyNum] = OPEN_LOCK_TIME;
                        }
                    } else {
                        if (pressState[keyNum] == 0) {
                            pressState[keyNum] = 1;  //1表示转动已经按下
                            trun_timer[keyNum] = TRUN_TIME;
                        }
                    }
                    break;

                case R.id.Hongkong3_mode:
                    keyNum = 2;
                    inputCheck = false;
                    Bundle bundle = new Bundle();
                    Intent intent = new Intent(HongKong.this, ModeSetActivity.class);
                    bundle.putInt("glass_num", keyNum);
                    intent.putExtras(bundle);
                    startActivity(intent);
                    break;
                default:
                    inputCheck = false;
                    break;
            }

            if(inputCheck){
                changeViewShade();
                glassOpenLock[keyNum].setEnabled(false); //禁止按下
                glassTrunOn[keyNum].setEnabled(false); //禁止按下
                process_bar[keyNum].setVisibility(View.VISIBLE);
                glassOpenLock[keyNum].setVisibility(View.VISIBLE);
                glassTrunOn[keyNum].setVisibility(View.VISIBLE);
                glassTopCicle[keyNum].setVisibility(View.INVISIBLE);
                glassBottomCicle[keyNum].setVisibility(View.INVISIBLE);

                temp_bytes[0] = (byte) 0x06;       //0x06 开锁命令
                temp_bytes[1] = keyNum; //地址 参数0  地址，LED模式，增加删除指纹ID,
                temp_bytes[4] = (byte) pressState[keyNum];//锁开关
                byte[] send = makeStringtoFramePackage(temp_bytes);
                /*串口发送字节*/
                try {
                    ttyS0OutputStream.write(send);
                    //ttyS1InputStream.read(send);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    };
}
