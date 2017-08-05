/**
 ******************************************************************************
 ** �t�@�C���� : app.cpp
 **
 ** �T�v : 2017/8/5�[�������U���p
 **
 ** ���L : 
 ******************************************************************************
 **/

#include "app.h"
#include "Run4mAndStop.h" 

using namespace ev3api;

// using�錾
using ev3api::GyroSensor;
using ev3api::TouchSensor;
using ev3api::Motor;

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

/* Bluetooth */
static int32_t   bt_cmd = 0;      /* Bluetooth�R�}���h 1:�����[�g�X�^�[�g */
static FILE     *bt = NULL;      /* Bluetooth�t�@�C���n���h�� */

/* ���L�̃}�N���͌�/���ɍ��킹�ĕύX����K�v������܂� */
#define GYRO_OFFSET           0  /* �W���C���Z���T�I�t�Z�b�g�l(�p���x0[deg/sec]��) */
#define LIGHT_WHITE          40  /* ���F�̌��Z���T�l */
#define LIGHT_BLACK           0  /* ���F�̌��Z���T�l */
#define SONAR_ALERT_DISTANCE 30  /* �����g�Z���T�ɂ���Q�����m����[cm] */
#define TAIL_ANGLE_STAND_UP  93  /* ���S��~���̊p�x[�x] */
#define TAIL_ANGLE_DRIVE      3  /* �o�����X���s���̊p�x[�x] */
#define P_GAIN             2.5F  /* ���S��~�p���[�^������W�� */
#define PWM_ABS_MAX          60  /* ���S��~�p���[�^����PWM��΍ő�l */
//#define DEVICE_NAME     "ET0"  /* Bluetooth�� hrp2/target/ev3.h BLUETOOTH_LOCAL_NAME�Őݒ� */
//#define PASS_KEY        "1234" /* �p�X�L�[    hrp2/target/ev3.h BLUETOOTH_PIN_CODE�Őݒ� */
#define CMD_START         '1'    /* �����[�g�X�^�[�g�R�}���h */

/* LCD�t�H���g�T�C�Y */
#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)

// �I�u�W�F�N�g��ÓI�Ɋm�ۂ���
GyroSensor  gGyroSensor(PORT_4);
TouchSensor gTouchSensor(PORT_1);
Motor       gLeftWheel(PORT_C);
Motor       gRightWheel(PORT_B);

//�I�u�W�F�N�g�̒�`
static Run4mAndStop *gRun4mAndStop;
static BeforeStart *gBeforeStart;
//static Start *Ev3Start;
//static Running *Ev3Running
static Stop *gStop;

static BalanceRunning *gBalanceRunning;


static void user_system_create()
{
	//�^�b�`�Z���T�������҂�
	tslp_tsk(2);

	gRun4mAndStop = new Run4mAndStop(gBeforeStart,gStop);
	gBeforeStart = new BeforeStart(gTouchSensor);
//	Ev3Start = new Ev3Start();
//	Ev3Running = new Ev3Running();
	gStop = new Stop(gBalanceRunning);

	gBalanceRunning = new BalanceRunning(gGyroSensor);
}

static void user_system_destroy()
{
	delete gRun4mAndStop;
	delete gBeforeStart;
//	delete Ev3Start;
//	delete Ev3Running;
	delete gStop;

	delete gBalanceRunning;

}


/* ���C���^�X�N */
void main_task(intptr_t unused)
{
	user_system_create();

	//�����n���h���J�n
	ev3_sta_cyc(EV3_CYC_ZEROYON);
		
		slp_tsk();  // �o�b�N�{�^�����������܂ő҂�

	// �����n���h����~
	ev3_stp_cyc(EV3_CYC_ZEROYON);

	user_system_destroy();

    ext_tsk();
}

//*****************************************************************************
//ev3_cyc_zeroyon�^�X�N
//
//�T��:zeroyon�^�X�N���쐬����
//
//*****************************************************************************

void ev3_cyc_zeroyon(intptr_t unused)
{
	act_tsk(ZEROYON_TASK);
}

//*****************************************************************************
//zeroyon�^�X�N
//
//�T��:��Ԃ̐؂�ւ����s��
//
//*****************************************************************************

void zeroyon_task(intptr_t exinf)
{
	if (ev3_button_is_pressed(BACK_BUTTON)) wup_tsk(MAIN_TASK);
	else gRun4mAndStop->runTask();

}

//*****************************************************************************
// �֐��� : bt_task
// ���� : unused
// �Ԃ�l : �Ȃ�
// �T�v : Bluetooth�ʐM�ɂ�郊���[�g�X�^�[�g�B Tera Term�Ȃǂ̃^�[�~�i���\�t�g����A
//       ASCII�R�[�h��1�𑗐M����ƁA�����[�g�X�^�[�g����B
//*****************************************************************************
void bt_task(intptr_t unused)
{
    while(1)
    {
        uint8_t c = fgetc(bt); /* ��M */
        switch(c)
        {
        case '1':
            bt_cmd = 1;
            break;
        default:
            break;
        }
        fputc(c, bt); /* �G�R�[�o�b�N */
    }
}
