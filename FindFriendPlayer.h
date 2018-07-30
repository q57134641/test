#pragma once
#include "GameLib/Game/Game/GamePlayer.h"
#include "GameLib/Game/Game/GameBase.h"
#include "GameLib/Game/GameLib.h"
#include "cocos2d.h"

#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "FindFriendCard.h"

#define ONEPLACENUM 3
#define TWOPLACENUM 3
#define THREEPLACENUM 3

#define SEG_COUNT                       3                               //����
#define MAX_SEG_COUNT                   3                              //ÿ������˿�
#define TAG_CHECK_IP					100								//�ӳټ��IP
#define TAG_SEND_EXP					101								//���ͱ���
#define TAG_CONTINUE_EXP				102								//10�����ͱ���

#define TAG_SEG_CARD_START				500								//ÿ�ն�����ʼTAG

//���ư�ť״̬
#define SEEN_BTN_ON						0
#define SEEN_BTN_OFF					1

#define MAX_HANDCARD_COUNT				38

using namespace cocos2d::ui;


class FindFriendGameScence;

class FindFriendPlayer:public GamePlayer
{
public:
	enum
	{
		STATUS_NULL, STATUS_FIRST, STATUS_SECOND, STATUS_THIRD, STATUS_FOURTH, STATUS_CALLFRIEND
	};

	enum 
	{
		CALL_NULL,MING,AN,FRIEND
	};
	
	FindFriendPlayer(int iIdex,cocos2d::Node* pSeatNode);
	~FindFriendPlayer();
	void init();
	int  getIdex();
	int getUserId();

	void defaultState();
	void hideHeadEmpty();
	void showheadEmpty();
	
	void setGameBase(GameBase* data){ m_gameScene = data; };
	void queryPlayerInfo();
	
	void EndGame();
	void recoverGame();

	void cleanAllInfo();
	
	//��ʾ����,name��1��ʼ
	void showExpression(DWORD name);
	//�Ƴ�����
	void cleanExpression();
	//���ſ������
	void playLocalAudio(DWORD name);
	//���س��ƿ�
	void hideOutCardLayer();
	//��ʾ��ʱ��λ��
	Point getTimerWorldPos();
//��ʾ�û���Ϣ
public:
	//�����û���Ϣ
	void sePlayerInfo(int iUserId, char szHeadHttp[], int iHeadLen);
	bool isShowHead();
	//�Ƿ���ʾ΢��ͷ��
	void setHeadVisible(bool bVisible);
	//����ͷ��
	void downloadHeadUrl(Sprite* head = nullptr);
	void setHeadDownload(bool bDownload){ m_bDownload = bDownload; }
	bool isHeadDownloaded(){ return m_bDownload; }
	//����ͷ��
	void resetHead();
	//��ȡͷ����������
	Point getHeadWorldPoint();
	//��ʾ�û���Ϣ
	void onBtnPlayerInfo(Ref* ref);
	void showPlayerInfo(bool bFriend);
	
	//���������Ϣ��
	void updatePlayerInfoBox();
	void hidePlayerInfoBox();
	void cleanPlayerInfo();
	//������ʾ
	void updateRoomMaster(dword dwCreateRoomId);
	//��ʼ�����鰴ť
	void initExpressionButton(Node* node);
	void onBtnSendExpression(Ref* ref);
	void setHeadInfo(const char* szHeadHttp,const char* szIp,int iCount, int iUserId);
	void setNickName(std::string sName);
	void setNickNameVisible(bool bVisible);
	void showFriendStatus(int iStatus);
	void cleanFriendStatus();
//�˿����
public:
	//��ʾ������Ϣ
	void showOutCard(BYTE bCard[], int iCount);
	//����������ʾ����Ч��
	void showOutCardAction(BYTE bCardData[],int iCount);
	void cleanOutCardAction();
	void showLeftCardCount(int iCount);
	int getLeftCardCount(){ return m_iLeftCardCount; }
	void hideLeftCount();
	void showPass();
	void playPassSound();
	void hidePass();
	void hideOutCardInfo();
	void setDogCard(BYTE bDogCard){ m_bDogCard = bDogCard; }
	//������������
	void playTypeSound(BYTE bOutCard[], int iCount);
	//��������
	void playLeftOneSound();
	//��ʾ�Ͼ�ʤ�����
	void showWinnerInfo(int iGrade);
	//��ʾ�͹��˿�
	void showTributeCard(BYTE bCard,BYTE bMainCard);
	//��ȡ�͹��˿���ʾλ��
	Point getTributeCardWorldPos();
	//��ʾ�����ɹ�
	void showNotTribute();
	void cleanNotTribute();
	//�����͹���Ϣ
	void cleanTributeInfo();
	void showEgg(int iGrade);
	void hideEgg();
	void setGrade(BYTE bGrade);
	void updateLeftCardBg(int iStatus);
	//����
public:
	//��ʾ��Ϸ�����÷�
	void showGameScore(LONGLONG lScore);
	void hideGameScore();
	void showXiScore(int iScore);
	//��ʾ��Ϸ�ܵ÷�
	void showTotalScore(LONGLONG lScore);
	void setTotalScoreVisible(bool bVisible);
	//��ʾ������Ϣ
	void showChatMsg(std::string strContent);
	void hideChatMsg();
public:
	virtual void PlayerEnter();
	virtual void PlayerLeave(bool real = false);
	virtual void upPlayerInfo();
	virtual void upPlayerState();
	void setLeaveImg(bool bState);
public:
	virtual void playVoiceAction(float time);
	void cleanVoiceAction();

public:
	//ֻ�����˿�
	void createCards(BYTE bCards[], int iCount);
	//�����˿�
	void calculateAndSortCards();
	void removeCards(BYTE bCardData[], int iCount);
	void clearCards();
	void addCards(BYTE bCardData[], int iCount);

	bool isHaveCard(BYTE bCardData);
	void SortCards();
	//�����е�������ɾ����
	void removeSortCard(FindFriendCard* card);
	//�����е������������
	void addSortCard(FindFriendCard* card[], int cardCount, bool isAuto);
	void calculateCardsVect();

private:
	int						m_iIdex;
	cocos2d::Node*			m_pSeatNode;
	GameBase* m_gameScene;
	cocos2d::Node*		m_headEmpty;//��ͷ��
	Sprite* m_ready;
	
	ui::ImageView* m_voiceActBg;
	float m_voiceTime;
	bool m_bVoiceAction;

	int m_iGender;											 //0�� 1Ů
	bool m_bReadyTime;
	bool m_bSendExpression;									 //�Ƿ��Ѿ����ͱ���

	int m_iUserId;												//�����û�ID
	int m_iGameID;												//������ϷID
	char m_szHeadHttp[LEN_USER_NOTE];							//����headHttp
	std::string m_sIP;											//�û�IP
	std::string m_sNickName;									//�û��ǳ�
	bool m_bDownload;											//�Ѿ�������ͷ��

	int m_iLeftCardCount;										//ʣ���˿���
	BYTE				m_bDogCard;
	BYTE				m_bGrade;

	cocos2d::Node*		m_handCardLayout;//��ͷ��
	//��ǰѡ��
	Vector<FindFriendCard*> m_vAllCards;
};

