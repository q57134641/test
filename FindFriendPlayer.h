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

#define SEG_COUNT                       3                               //墩数
#define MAX_SEG_COUNT                   3                              //每墩最多扑克
#define TAG_CHECK_IP					100								//延迟检测IP
#define TAG_SEND_EXP					101								//发送表情
#define TAG_CONTINUE_EXP				102								//10连发送表情

#define TAG_SEG_CARD_START				500								//每墩动画起始TAG

//明牌按钮状态
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
	
	//显示表情,name从1开始
	void showExpression(DWORD name);
	//移除表情
	void cleanExpression();
	//播放快捷语音
	void playLocalAudio(DWORD name);
	//隐藏出牌框
	void hideOutCardLayer();
	//显示定时器位置
	Point getTimerWorldPos();
//显示用户信息
public:
	//设置用户信息
	void sePlayerInfo(int iUserId, char szHeadHttp[], int iHeadLen);
	bool isShowHead();
	//是否显示微信头像
	void setHeadVisible(bool bVisible);
	//下载头像
	void downloadHeadUrl(Sprite* head = nullptr);
	void setHeadDownload(bool bDownload){ m_bDownload = bDownload; }
	bool isHeadDownloaded(){ return m_bDownload; }
	//重置头像
	void resetHead();
	//获取头像世界坐标
	Point getHeadWorldPoint();
	//显示用户信息
	void onBtnPlayerInfo(Ref* ref);
	void showPlayerInfo(bool bFriend);
	
	//更新玩家信息框
	void updatePlayerInfoBox();
	void hidePlayerInfoBox();
	void cleanPlayerInfo();
	//房主显示
	void updateRoomMaster(dword dwCreateRoomId);
	//初始化表情按钮
	void initExpressionButton(Node* node);
	void onBtnSendExpression(Ref* ref);
	void setHeadInfo(const char* szHeadHttp,const char* szIp,int iCount, int iUserId);
	void setNickName(std::string sName);
	void setNickNameVisible(bool bVisible);
	void showFriendStatus(int iStatus);
	void cleanFriendStatus();
//扑克相关
public:
	//显示出牌信息
	void showOutCard(BYTE bCard[], int iCount);
	//特殊牌型显示动画效果
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
	//播放牌型语音
	void playTypeSound(BYTE bOutCard[], int iCount);
	//报单声音
	void playLeftOneSound();
	//显示上局胜利情况
	void showWinnerInfo(int iGrade);
	//显示送贡扑克
	void showTributeCard(BYTE bCard,BYTE bMainCard);
	//获取送贡扑克显示位置
	Point getTributeCardWorldPos();
	//显示抗贡成功
	void showNotTribute();
	void cleanNotTribute();
	//清理送贡信息
	void cleanTributeInfo();
	void showEgg(int iGrade);
	void hideEgg();
	void setGrade(BYTE bGrade);
	void updateLeftCardBg(int iStatus);
	//分数
public:
	//显示游戏结束得分
	void showGameScore(LONGLONG lScore);
	void hideGameScore();
	void showXiScore(int iScore);
	//显示游戏总得分
	void showTotalScore(LONGLONG lScore);
	void setTotalScoreVisible(bool bVisible);
	//显示聊天信息
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
	//只创建扑克
	void createCards(BYTE bCards[], int iCount);
	//排序扑克
	void calculateAndSortCards();
	void removeCards(BYTE bCardData[], int iCount);
	void clearCards();
	void addCards(BYTE bCardData[], int iCount);

	bool isHaveCard(BYTE bCardData);
	void SortCards();
	//从排列的向量里删除牌
	void removeSortCard(FindFriendCard* card);
	//向排列的向量里添加牌
	void addSortCard(FindFriendCard* card[], int cardCount, bool isAuto);
	void calculateCardsVect();

private:
	int						m_iIdex;
	cocos2d::Node*			m_pSeatNode;
	GameBase* m_gameScene;
	cocos2d::Node*		m_headEmpty;//空头像
	Sprite* m_ready;
	
	ui::ImageView* m_voiceActBg;
	float m_voiceTime;
	bool m_bVoiceAction;

	int m_iGender;											 //0男 1女
	bool m_bReadyTime;
	bool m_bSendExpression;									 //是否已经发送表情

	int m_iUserId;												//保存用户ID
	int m_iGameID;												//保存游戏ID
	char m_szHeadHttp[LEN_USER_NOTE];							//保存headHttp
	std::string m_sIP;											//用户IP
	std::string m_sNickName;									//用户昵称
	bool m_bDownload;											//已经下载完头像

	int m_iLeftCardCount;										//剩余扑克数
	BYTE				m_bDogCard;
	BYTE				m_bGrade;

	cocos2d::Node*		m_handCardLayout;//空头像
	//当前选中
	Vector<FindFriendCard*> m_vAllCards;
};

