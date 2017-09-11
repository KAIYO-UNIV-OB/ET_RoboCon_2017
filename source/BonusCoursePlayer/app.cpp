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
#include "BonusCoursePlayer.h" 
#include "DebugStarter.h"
#include "unit/BalancingRunner.h"
#include "unit/StairPlayer.h"
#include "unit/GaragePlayer.h"
#include "unit/LookUpGatePlayer.h"
#include "unit/CalculateRunningDirection.h"
#include "GyroSensor.h"
#include "TouchSensor.h"
#include "ColorSensor.h"
#include "SonarSensor.h"
#include "Motor.h"

#include "BlueToothCommunication.h"

void *__dso_handle = 0;

// using�錾
using ev3api::GyroSensor;
using ev3api::TouchSensor;
using ev3api::ColorSensor;
using ev3api::SonarSensor;
using ev3api::Motor;

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

/* Bluetooth */
//static int32_t   bt_cmd = 0;      /* Bluetooth�R�}���h 1:�����[�g�X�^�[�g */
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
TouchSensor gTouchSensor(PORT_1);
SonarSensor gSonarSensor(PORT_2);
ColorSensor gColorSensor(PORT_3);
GyroSensor  gGyroSensor(PORT_4);

Motor		gTailMotor(PORT_A);
Motor       gRightMotor(PORT_B);
Motor       gLeftMotor(PORT_C);

//�I�u�W�F�N�g�̒�`
static unit::BalancingCalculator	*gBalancingCalculator;
static unit::BalancingRunner		*gBalancingRunner;
static unit::BalancingRunnerZERO	*gBalancingRunnerZERO;
static unit::BatteryMonitor			*gBatteryMonitor;
static BonusCoursePlayer			*gBonusCoursePlayer;
static unit::ColorJudge				*gColorJudge;
static unit::DebugStarter			*gDebugStarter;
static unit::PostureController		*gPostureController;
static unit::TouchSensorMonitor		*gTouchSensorMonitor;
static unit::FindStair *gFindStair;
static unit::IAnalogMeter *gIAnalogMeter;
static unit::DistanceMeter *gDistanceMeter;
static unit::TailMovement *gTailMovement;
static unit::MotorController		*gMotorController;
static unit::GyroMonitor			*gGyroMonitor;
static unit::GaragePlayer			*gGaragePlayer;
static unit::LookUpGatePlayer		*gLookUpGatePlayer;
static unit::CalculateRunningDirection		*gCalculateRunningDirection;
static unit::SelfPositionChecker	*gSelfPositionChecker;
static unit::SonarMonitor *gSonarMonitor;
static unit::StairPlayer			*gStairPlayer;
static unit::StartMonitor			*gStartMonitor;



static	unit::BlueToothCommunication *gBlueToothCommunication;

static void user_system_create()
{
	//�^�b�`�Z���T�������҂�
	tslp_tsk(2);

	//�f�o�b�O�pCOM�|�[�g���j�^
	gBlueToothCommunication = new unit::BlueToothCommunication(bt, gTouchSensorMonitor, gGyroMonitor);


	//���̃C���X�^���X�ŌĂԂ��̂͐�ɐ錾����
	gBatteryMonitor			= new unit::BatteryMonitor();
	gBalancingCalculator	= new unit::BalancingCalculator();
	gMotorController		= new unit::MotorController();
	gGyroMonitor			= new unit::GyroMonitor();
	gSonarMonitor = new unit::SonarMonitor();
	gBalancingRunner = new unit::BalancingRunner(gMotorController, gGyroMonitor);
	gBalancingRunnerZERO = new unit::BalancingRunnerZERO(gMotorController, gGyroMonitor);
	gTouchSensorMonitor		= new unit::TouchSensorMonitor();
	gPostureController		= new unit::PostureController();
	gTailMovement = new unit::TailMovement(gBalancingRunnerZERO, gMotorController);
	
	gDistanceMeter = new unit::DistanceMeter(0,0);

	gFindStair = new unit::FindStair(gGyroMonitor);
	gColorJudge = new unit::ColorJudge(gTouchSensorMonitor);
	gSelfPositionChecker = new unit::SelfPositionChecker(gColorJudge);
	gCalculateRunningDirection = new unit::CalculateRunningDirection(gSelfPositionChecker);

	gStartMonitor = new unit::StartMonitor(gTouchSensorMonitor);
	gDebugStarter = new unit::DebugStarter(gPostureController, gStartMonitor, gBalancingRunner, gBalancingRunnerZERO, gDistanceMeter, gMotorController);
	gStairPlayer = new unit::StairPlayer(gBalancingRunnerZERO, gCalculateRunningDirection, gFindStair, gTailMovement, gDistanceMeter, gMotorController);
	gLookUpGatePlayer = new unit::LookUpGatePlayer(gBalancingRunnerZERO, gDistanceMeter, gMotorController,gTailMovement, gSonarMonitor);
	gGaragePlayer = new unit::GaragePlayer(gBalancingRunnerZERO, gCalculateRunningDirection, gDistanceMeter, gMotorController, gTailMovement);
	gBonusCoursePlayer = new BonusCoursePlayer(gDebugStarter, gStairPlayer, gLookUpGatePlayer, gGaragePlayer);

}

static void user_system_destroy()
{
	delete gIAnalogMeter;
	delete gDistanceMeter;
	delete gBatteryMonitor;
	delete gBalancingCalculator;
	delete gBalancingRunner;
	delete gFindStair;
	delete gGyroMonitor;
	delete gMotorController;
	delete gBonusCoursePlayer;
	delete gDebugStarter;
	delete gTouchSensorMonitor;
	delete gStartMonitor;
	delete gPostureController;
	delete gStairPlayer;
	delete gSonarMonitor;
	delete gLookUpGatePlayer;
	delete gGaragePlayer;
	delete gBalancingRunnerZERO;
	delete gTailMovement;

	delete gBlueToothCommunication;

}


/* ���C���^�X�N */
void main_task(intptr_t unused)
{
	user_system_create();

	//�����n���h���J�n
	ev3_sta_cyc(EV3_CYC_ZEROYON);
	ev3_sta_cyc(EV3_CYC_COM);
		
		slp_tsk();  // �o�b�N�{�^�����������܂ő҂�

	// �����n���h����~
	ev3_stp_cyc(EV3_CYC_ZEROYON);
	ev3_stp_cyc(EV3_CYC_COM);

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

void ev3_cyc_com(intptr_t unused)
{
	act_tsk(BT_TASK);
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
	else gBonusCoursePlayer->runTask();

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
	gBlueToothCommunication->execute();
	

}
