#include "FindFriendPlayer.h"
#include "HNMJSoundFun.h"
#include "SimpleAudioEngine.h"
#include "Classes/ClientHN_THJ/Common/Common.h"
#include "CMD_FindFriend.h"
#include "ClientHN_THJ/Define/SettingDefine.h"
#include "ClientHN_THJ/Define/AnimationDefine.h"
#include "JniCross/JniFun.h"
#include "FindFriendLogic.h"
#include <spine/spine-cocos2dx.h>

USING_NS_CC;
#define AVATAR_SIZE 50

#define TAG_ACTION_HIDE_MSG			1234
#define TAG_ACTIN_SHOW_HEAD			1235			//延迟显示头像动作
#define TIME_SHOW_HEAD				18				//延迟显示头像事件
#define CARD_SCALE					0.3

#define TIME_SEEN_CARD_ACTION		0.3
#define MAX_OUT_CARD_LINE			7
#define REPLAY_CARD_SCALE			0.35			//回放扑克缩放大小

FindFriendPlayer::FindFriendPlayer(int iIdex, cocos2d::Node* pSeatNode)
	:GamePlayer(NULL)
	, m_iIdex(iIdex)
	, m_pSeatNode(pSeatNode)
	, m_headEmpty(nullptr)
	, m_voiceActBg(nullptr)
	, m_bVoiceAction(false)
	, m_ready(nullptr)
	, m_voiceTime(0.0)
	, m_iGender(0)
	, m_gameScene(nullptr)
	, m_bReadyTime(false)
	, m_bSendExpression(false)
{
	m_iUserId = 0;										
	m_iGameID = 0;
	zeromemory(m_szHeadHttp, sizeof(m_szHeadHttp));
	m_sIP ="";
	m_sNickName = "";

	m_bDownload = false;
	m_iLeftCardCount = 0;
	m_bGrade = 0;
	init();
}

FindFriendPlayer::~FindFriendPlayer()
{
	
}

void FindFriendPlayer::init()
{
	m_headEmpty =m_pSeatNode->getChildByName("headEmpty");
	if (!m_headEmpty) return;

	m_headEmpty->setVisible(false);
	setHeadVisible(false);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto winSize = Director::getInstance()->getOpenGLView()->getDesignResolutionSize();

	
	m_pSeatNode->setScale(MIN(visibleSize.width / winSize.width, visibleSize.height / winSize.height));
	if (m_iIdex == 0)
	{
		m_pSeatNode->setPositionX(Director::getInstance()->getVisibleSize().width / 2);
		m_pSeatNode->setZOrder(1);
	}
	
	

	m_voiceActBg = dynamic_cast<ui::ImageView*>(m_pSeatNode->getChildByName("voice_bugle_bg"));
	if (m_voiceActBg)
	{
		//m_voiceActBg->setContentSize(Size(150, 67));
		m_voiceActBg->setGlobalZOrder(1000);
		m_voiceActBg->setVisible(false);
	}

	m_ready = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("ready"));
	m_ready->setVisible(false);

	auto btnHead = dynamic_cast<ui::Button*>(m_headEmpty->getChildByName("head_btn"));
	if (btnHead)
	{
		btnHead->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnPlayerInfo, this));
	}

	m_iGender = GetGender();
	hideGameScore();
	hideLeftCount();
	hideOutCardLayer();
	hidePass();
	showTotalScore(0);

	auto chatBg = dynamic_cast<ui::ImageView*>(m_headEmpty->getChildByName("chatBg"));
	if (chatBg)
	{
		chatBg->setVisible(false);
		chatBg->setGlobalZOrder(2100);
	}
		
	auto roomMaster = m_headEmpty->getChildByName("roomMaster");
	if (roomMaster)
		roomMaster->setVisible(false);

	hideChatMsg();

	resetHead();
	
	auto btnOpenCard = dynamic_cast<ui::Button*>(m_pSeatNode->getChildByName("btnOpenCard"));
	if (btnOpenCard)
		btnOpenCard->setVisible(false);

	m_handCardLayout = (m_pSeatNode->getChildByName("handCardLayout"));
	auto leftCard = m_pSeatNode->getChildByName("leftCard");
	if (leftCard)
		leftCard->setVisible(false);
}
int  FindFriendPlayer::getIdex()
{
	return m_iIdex;
}
void FindFriendPlayer::defaultState()
{
	//m_voiceActBg->setVisible(false);
	//m_bVoiceAction = false;
	m_ready->setVisible(false);
	if (m_pSeatNode)
		m_pSeatNode->stopAllActions();

	m_bSendExpression = false;
	hideGameScore();
	cleanExpression();
	hideLeftCount();
	hideChatMsg();
	hidePass();
	upPlayerState();
	hideOutCardInfo();
	cleanOutCardAction();
	hideEgg();
	cleanNotTribute();
	showWinnerInfo(0);
	cleanFriendStatus();
}

void FindFriendPlayer::EndGame()
{
}

void FindFriendPlayer::PlayerEnter()
{
	if (!getUserItem()) return;
	
	m_iGender = GetGender();
	cocos2d::log("checkHeadEmpty:PlayerEnter");
	int iChair = GetChairID();
	
	if (m_pSeatNode)
	{
		m_pSeatNode->stopActionByTag(TAG_ACTIN_SHOW_HEAD);
		Sequence* seqShowHead = Sequence::create(DelayTime::create(TIME_SHOW_HEAD), CallFunc::create([=](){
			setHeadDownload(true);
			setHeadVisible(true);
		}), NULL);
		m_pSeatNode->runAction(seqShowHead);
		seqShowHead->setTag(TAG_ACTIN_SHOW_HEAD);
	}
	
	showheadEmpty();
	
	Sequence* seq = Sequence::create(DelayTime::create(1.5f), CallFunc::create([=](){
		if (m_gameScene)
			m_gameScene->checkSameIp();
	}), NULL);

	m_pSeatNode->stopActionByTag(TAG_CHECK_IP);
	seq->setTag(TAG_CHECK_IP);
	m_pSeatNode->runAction(seq);
}

void FindFriendPlayer::PlayerLeave(bool real)
{
	if (!getUserItem()) return;

	if (m_pSeatNode)
	{
		m_pSeatNode->stopActionByTag(TAG_ACTIN_SHOW_HEAD);
		m_pSeatNode->stopActionByTag(TAG_CHECK_IP);
	}
	
	cocos2d::log("checkRecover:PlayerLeave");
	
	if (m_ready)
		m_ready->setVisible(false);
	if (m_headEmpty)
	{
		m_headEmpty->setVisible(false);
	}

	/*if (m_gameScene&&m_gameScene->getServerType() == GAME_GENRE_GOLD)
		real = true;*/
		
	if (m_gameScene)
	{
		if (!real || (m_gameScene->isRecoverGame() && m_gameScene->isResetUserItem()))
		{
			m_headEmpty->setVisible(true);
		}
		
		//m_gameScene->upHeadState(GetChairID(), false);
	}

	if (real)
		cleanAllInfo();
}

void FindFriendPlayer::upPlayerInfo()
{
	if (!getUserItem()) return;
	if (!m_headEmpty) return;

	//CommonMethod::cutNickName(strNickName, 16);
	auto nameText = dynamic_cast<ui::Text*>(m_headEmpty->getChildByName("name"));
	if (nameText)
		nameText->setString(utility::cutString(GetNickName(), 7, ".."));
	
	//头像
	auto avatar = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("sprWeixin"));
	if (avatar)
	{
		avatar->setLocalZOrder(0);
		auto roomMaster = m_headEmpty->getChildByName("roomMaster");
		if (roomMaster)
			roomMaster->setLocalZOrder(1);

		downloadHeadUrl(avatar);
	}
	
	if (m_gameScene&&m_gameScene->getServerType() == GAME_GENRE_GOLD){
		showTotalScore(GetUserScore());
	}
}
void FindFriendPlayer::upPlayerState()
{
	if (m_gameScene)
	{
		/*if (m_gameScene->isRecoverGame())
		{
		return;
		}*/
	}

	if (!getUserItem()) return;

	if (GetUserStatus() == US_SIT)
	{
		//准备
		setLeaveImg(false);
		m_ready->setVisible(false);
	}
	if (GetUserStatus() == US_READY)
	{
		//准备
		setLeaveImg(false);
		m_ready->setVisible(true);
		
		if (m_iIdex == 0)
		{
			m_gameScene->setReadyVisible(false);
			m_gameScene->setSendReadyState(false);
		}		
		hidePass();
		hideEgg();
		hideOutCardLayer();
		hideGameScore();
		clearCards();
	}
	
	if (GetUserStatus() == US_PLAYING)
	{
		//准备
		setLeaveImg(false);

		m_ready->setVisible(false);

		if (m_iIdex == 0)
		{
			m_gameScene->setReadyVisible(false);
			m_gameScene->setSendReadyState(false);
		}
	}
	if (GetUserStatus() == US_OFFLINE)
	{
		//断线
		setLeaveImg(true);
	}
}

void FindFriendPlayer::recoverGame()
{
	m_bVoiceAction = false;
	
	cleanExpression();
	hideChatMsg();
}

void FindFriendPlayer::playVoiceAction(float time)
{
	if (m_bVoiceAction)
		return;
	m_bVoiceAction = true;

	m_voiceTime = time;

	if (!m_voiceActBg)
		return;

	m_voiceActBg->setVisible(true);
	
	auto timeText = dynamic_cast<ui::Text*>(m_voiceActBg->getChildByName("time"));
	if (!timeText)
		return;

	m_voiceActBg->stopAllActions();

	std::string timeStr = utility::toString(m_voiceTime);
	char name[20];
	sprintf(name, "%0.1f\"", m_voiceTime);
	//timeText->setText(name);
	timeText->setText("");

	if (timeText)
	{
		timeText->setPosition(Point(m_voiceActBg->getContentSize().width - timeText->getContentSize().width - 10, m_voiceActBg->getContentSize().height / 2));
	}

	m_pSeatNode->unschedule("UpdateTime");
	m_pSeatNode->schedule([=](float dt)
	{

		m_voiceTime -= dt;
		if (m_voiceTime >= 0.0f)
		{
			char name[20];
			sprintf(name, "%0.1f\"", m_voiceTime);
			//timeText->setText(name);
			timeText->setText("");
		}
		else
		{
			std::string timeStr = utility::toString(0) + "\"";
			timeText->setText("");
			m_pSeatNode->unschedule("UpdateTime");
			m_bVoiceAction = false;
			
			m_voiceActBg->stopAllActions();
			m_voiceActBg->setVisible(false);
			m_voiceTime = 0.0f;
		}

	}, "UpdateTime");

	auto timeCallBack = [=]()
	{
		std::string timeStr = utility::toString(time);
		//timeText->setText(timeStr);
		timeText->setText("");

	};
	
	//动画效果
	auto actImg = dynamic_cast<ui::ImageView*>(m_voiceActBg->getChildByName("actImg"));
	if (actImg)
		actImg->loadTexture("findFriend_game/voice/yuyin_xiao_1.png");

	DelayTime* delay = DelayTime::create(0.35f);
	auto show2CallBack = [=]()
	{
		if (actImg)
			actImg->loadTexture("findFriend_game/voice/yuyin_xiao_2.png");
	};
	auto show3CallBack = [=]()
	{
		if (actImg)
			actImg->loadTexture("findFriend_game/voice/yuyin_xiao_3.png");
	};

	auto showRipple2 = CallFunc::create(show2CallBack);
	auto showRipple3 = CallFunc::create(show3CallBack);

	auto hideAllCallBack = [=]()
	{
		if (actImg)
			actImg->loadTexture("findFriend_game/voice/yuyin_xiao_1.png");
	};

	auto hideAll = CallFunc::create(hideAllCallBack);

	Sequence* seq = Sequence::create(delay, showRipple2, delay, showRipple3, delay, hideAll, NULL);
	RepeatForever* rep = RepeatForever::create(seq);

	m_voiceActBg->runAction(rep);
}

void FindFriendPlayer::queryPlayerInfo()
{
	if (m_gameScene)
		m_gameScene->queryPlayerInfo();
}

void FindFriendPlayer::setLeaveImg(bool bState)
{
	auto leave = dynamic_cast<ui::ImageView*>(m_headEmpty->getChildByName("leaveImg"));
	if (leave)
		leave->setVisible(bState);
}

void FindFriendPlayer::cleanAllInfo()
{
	//m_voiceActBg->setVisible(false);
	//m_bVoiceAction = false;
	m_ready->setVisible(false);

	hideGameScore();
	cleanExpression();
	hideChatMsg();
	hideChatMsg();
	hideLeftCount();
	hideOutCardLayer();
	hidePass();
	showTotalScore(0);
	cleanOutCardAction();
	cleanTributeInfo();
	hideEgg();
	cleanNotTribute();
	clearCards();
	cleanFriendStatus();

	m_iGameID = 0;
	m_iUserId = 0;
	m_sIP = "";
	zeromemory(m_szHeadHttp, sizeof(m_szHeadHttp));
	m_sNickName = "";

	resetHead();
}

void FindFriendPlayer::showExpression(DWORD name)
{
	ASSERT(name <= MAX_EXPRESSION_COUNT &&name>=0);
	if (name > MAX_EXPRESSION_COUNT || name <0) return;

	auto face = m_headEmpty->getChildByName("face");
	if (!face)
		return;

	auto sprWeixin = m_headEmpty->getChildByName("sprWeixin");
	if (!sprWeixin)
		return;

	face->setVisible(true);

	face->removeAllChildren();
	face->setPosition(sprWeixin->getPosition());
	face->setLocalZOrder(1000);

	std::string sEmotionName = "Emotion_" + utility::toString(name);
	auto animation = AnimationCache::getInstance()->getAnimation(sEmotionName);
	if (!animation) return;

	auto sp = Sprite::create();
	if (sp)
	{
		face->addChild(sp);
		sp->setPosition(face->getContentSize() / 2);
	
		auto animate = Animate::create(animation);
		auto seq = Sequence::create(animate, RemoveSelf::create(), NULL);
		sp->runAction(seq);
	}
}

void FindFriendPlayer::cleanExpression()
{
	auto face = m_headEmpty->getChildByName("face");
	if (!face)
		return;
	face->stopAllActions();
	face->setVisible(false);
}

void FindFriendPlayer::onBtnPlayerInfo(Ref* ref)
{
	showPlayerInfo(true);
}

void FindFriendPlayer::showPlayerInfo(bool bFriend)
{
	auto scene = Director::getInstance()->getRunningScene();
	if (!scene)
		return;

	if (m_gameScene)
		m_gameScene->hidePlayersInfoBox();

	if (!m_pSeatNode) return;

	std::string sChildName = "nodePlayerInfo";
	sChildName = sChildName + utility::toString(m_iIdex);
	auto nodePlayerInfo = scene->getChildByName(sChildName);
	if (!nodePlayerInfo)
	{
		nodePlayerInfo = CSLoader::createNode("findFriend_game/nodePlayerInfo.csb");
		if (nodePlayerInfo)
		{
			nodePlayerInfo->setName("nodePlayerInfo");
			scene->addChild(nodePlayerInfo);
		}
	}
		
	if (!nodePlayerInfo)
		return;

	nodePlayerInfo->setVisible(true);
	nodePlayerInfo->setPosition(Director::getInstance()->getWinSize()/2);

	CommonMethod::initInfoTouchEvent(nodePlayerInfo, "playerInfoBg");
	initExpressionButton(nodePlayerInfo);

	if (m_gameScene->IsMatchGame()){
		return;
	}

	auto bg = nodePlayerInfo->getChildByName("playerInfoBg");
	if (!bg)
		return;

	auto nickName = GetNickName();
	if (nickName == "")
		nickName = m_sNickName;

	auto textName = dynamic_cast<ui::Text*>(bg->getChildByName("textName"));
	if (textName)
		textName->setString(nickName);

	auto lLove = GetUserLostCount();
	std::string strLove = CommonMethod::converGold(lLove);
	auto txtMeiLi = dynamic_cast<ui::Text*>(bg->getChildByName("txtMeiLi"));
	if (txtMeiLi)
		txtMeiLi->setString(strLove);

	auto lScore = GetUserScore();
	std::string strGold = CommonMethod::converGold(lScore);
	auto txtGold = dynamic_cast<ui::Text*>(bg->getChildByName("txtGold"));
	if (txtGold)
		txtGold->setString(strGold);

	auto gender = dynamic_cast<ui::ImageView*>(bg->getChildByName("gender"));
	if (gender)
	{
		gender->setVisible(true);
		gender->ignoreContentAdaptWithSize(true);
		if (m_iGender)
			gender->loadTexture("Common/personInfo/dating_gerenxinxi_touxiangnv_icon.png");
		else
			gender->loadTexture("Common/personInfo/dating_gerenxinxi_touxiangnan_icon.png");
	}
	
	auto textIP = dynamic_cast<ui::Text*>(bg->getChildByName("textIP"));
	
	if (m_sIP == ""){
		if (textIP)
			textIP->setString("***");
	}
	else{
		if (textIP)
			textIP->setString(m_sIP);
	}
		
	int iGameID = GetGameID();
	if (iGameID == 0)
		iGameID = m_iGameID;

	auto textId = dynamic_cast<ui::Text*>(bg->getChildByName("textId"));
	if (textId)
		textId->setString(utility::toString(iGameID));

	auto btnClose = dynamic_cast<ui::Button*>(bg->getChildByName("btnClose"));
	if (btnClose)
	{
		btnClose->setPressedActionEnabled(true);
		btnClose->addClickEventListener([=](Ref* ref){
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/ui_click.mp3");
			nodePlayerInfo->removeFromParent();
		});
	}
	auto btnReport = dynamic_cast<ui::Button*>(bg->getChildByName("btnReport"));
	if (btnReport)
	{
		btnReport->setPressedActionEnabled(true);
		btnReport->addClickEventListener([=](Ref* ref){
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/ui_click.mp3");
			//nodePlayerInfo->removeFromParent();
		});
		btnReport->setVisible(UserInfo::Instance().getUserID() != GetUserID());
		//btnReport->setVisible(false);
	}
	
	int total = GetUserPlayCount();
	int win = GetUserWinCount();
	int lost = GetUserLostCount();
	int free = GetUserFleeCount();
	auto txtWinPercent = dynamic_cast<ui::Text*>(bg->getChildByName("txtWinPercent"));
	if (txtWinPercent){
		if (total > 0){
			txtWinPercent->setString(CCString::createWithFormat("%.01f%%", float(win * 100 / total))->getCString());
		}
		else{
			txtWinPercent->setString("0.0%");
		}
		
		
	}

	auto txtEscapePercent = dynamic_cast<ui::Text*>(bg->getChildByName("txtEscapePercent"));
	if (txtEscapePercent){
		if (total > 0){
			txtEscapePercent->setString(CCString::createWithFormat("%.01f%%", float(free * 100 / total))->getCString());
		}
		else{
			txtEscapePercent->setString("0.0%");
		}
	}

	auto txtTotalMsg = dynamic_cast<ui::Text*>(bg->getChildByName("txtTotalMsg"));
	if (txtTotalMsg){
		std::string tmp = "(" + utility::toString(win) + CommonMethod::getChineseContent("wordSheng") + " "
			+ utility::toString(lost) + CommonMethod::getChineseContent("wordFu") + " "
			+ utility::toString(free) + CommonMethod::getChineseContent("wordTaoPao") + ")";
		txtTotalMsg->setString(tmp);
	}
		

	
	auto head = bg->getChildByName("head");
	auto headBg = bg->getChildByName("headBg");
	if (head&&headBg)
	{
		headBg->setLocalZOrder(100);
		
		if (GetUserID() == 0)
		{
			/*if (m_iUserId != 0 && strcmp(m_szHeadHttp, "") != 0)
				ImagicDownManager::Instance().addDown(head, m_szHeadHttp, m_iUserId);*/
			CommonMethod::cutFaceCircle(head, m_iUserId, m_szHeadHttp, 56, "Common/personInfo/dating_gerenxinxi_touxiangwaichen.png");
		}
		else
		{
			/*if (GetHeadHttp() != "")
				ImagicDownManager::Instance().addDown(head, GetHeadHttp(), GetUserID());*/
			CommonMethod::cutFaceCircle(head, GetUserID(), GetHeadHttp(), 56, "Common/personInfo/dating_gerenxinxi_touxiangwaichen.png");
		}

		head->setScale(headBg->getContentSize().height / head->getContentSize().height*0.8);
		head->setPosition(headBg->getPosition());
	}

	
}

void FindFriendPlayer::cleanPlayerInfo()
{
	auto scene = Director::getInstance()->getRunningScene();
	if (!scene) return;

	std::string sChildName = "nodePlayerInfo";
	sChildName = sChildName + utility::toString(m_iIdex);
	auto nodePlayerInfo = scene->getChildByName(sChildName);
	if (nodePlayerInfo)
		nodePlayerInfo->removeFromParent();
}

void FindFriendPlayer::playLocalAudio(DWORD name)
{
	int iGender = m_iGender;
	std::string soundName = "";

	std::string strVoice = UserDefault::getInstance()->getStringForKey("settingVoice", "default");
	cocos2d::log("CheckQuickMsg:strVoice:%s",strVoice.c_str());
	if (strVoice == "default")
	{
		cocos2d::log("CheckQuickMsg:==defalut");
		if (!m_iGender)
			soundName = "sound/audio/M/";
		else
			soundName = "sound/audio/W/";
	}
	else if (strVoice == "man")
	{
		soundName = "sound/audio/M/";
	}
	else if (strVoice == "woman")
	{
		soundName = "sound/audio/W/";
	}

	int iIndx = name - CHAT_MSG_START;

	soundName = soundName + "CHAT" + utility::toString(iIndx) + ".mp3";
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(soundName.c_str());
}

void FindFriendPlayer::showChatMsg(std::string strContent)
{
	if (!m_headEmpty)
		return;

	auto chatBg = dynamic_cast<ui::ImageView*>(m_headEmpty->getChildByName("chatBg"));
	if (!chatBg) return;

	chatBg->setVisible(true);

	auto text = dynamic_cast<ui::Text*>(chatBg->getChildByName("text"));
	if (!text) return;

	//text->setString(utility::to_u8(strContent));
	//text->setString("...");
	////默认=true
	//bool bIgnore = text->isIgnoreContentAdaptWithSize();
	////置为false,调用getAutoRenderSize().重新设置_label的大小(text真实大小)
	//text->ignoreContentAdaptWithSize(false);
	//Size sizeLabel = text->getAutoRenderSize();

	//float fTrueWidth = sizeLabel.width;
	//
	//float fWidth = fTrueWidth + 50;
	//text->setContentSize(Size(fTrueWidth,text->getContentSize().height));

	//Size contentSize = Size(fWidth, chatBg->getContentSize().height);
	//chatBg->setContentSize(contentSize);

	CallFunc* callFunc1 = CallFunc::create([=](){
		text->setString(".");
	});

	CallFunc* callFunc2 = CallFunc::create([=](){
		text->setString("..");
	});
	CallFunc* callFunc3 = CallFunc::create([=](){
		text->setString("...");
	});

	Sequence* seq = Sequence::create(DelayTime::create(0.5f), callFunc1, DelayTime::create(0.5f), callFunc2, DelayTime::create(0.5f), callFunc3, NULL);
	RepeatForever* repeat = RepeatForever::create(seq);
	if (repeat)
	{
		chatBg->stopAllActions();
		chatBg->runAction(repeat);
	}

	Sequence* seqHideMsg = Sequence::create(DelayTime::create(3.0f), CallFunc::create([=](){
		hideChatMsg();
	}), NULL);

	m_pSeatNode->stopActionByTag(TAG_ACTION_HIDE_MSG);
	seqHideMsg->setTag(TAG_ACTION_HIDE_MSG);
	m_pSeatNode->runAction(seqHideMsg);
}

void FindFriendPlayer::hideChatMsg()
{
	if (!m_headEmpty)
		return;

	auto chatBg = dynamic_cast<ui::ImageView*>(m_headEmpty->getChildByName("chatBg"));
	if (chatBg)
	{
		chatBg->stopAllActions();
		chatBg->setVisible(false);
	}

	auto text = dynamic_cast<ui::Text*>(chatBg->getChildByName("text"));
	if (text)
		text->setString("");
}

void FindFriendPlayer::updateRoomMaster(dword dwCreateRoomId)
{
	if (!m_headEmpty) return;

	auto roomMaster = m_headEmpty->getChildByName("roomMaster");
	if (!roomMaster) return;

	if (getUserId() == dwCreateRoomId)
		roomMaster->setVisible(true);
	else 
		roomMaster->setVisible(false);
}


Point FindFriendPlayer::getHeadWorldPoint()
{
	if (!m_pSeatNode || !m_headEmpty) return Point(0, 0);

	auto face = m_headEmpty->getChildByName("face");
	if (!face) return Point(0, 0);

	Point pos = m_headEmpty->convertToWorldSpace(face->getPosition());
	return pos;
}

void FindFriendPlayer::initExpressionButton(Node* node)
{
	if (!node) return;

	auto bg = node->getChildByName("playerInfoBg");
	if (!bg) return;

	//炸弹
	auto btnBomb = dynamic_cast<ui::Button*>(bg->getChildByName("btnBomb"));
	if (btnBomb)
	{
		btnBomb->setPressedActionEnabled(true);
		btnBomb->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnBomb->setTag(CHAT_BOMB);
	}

	//瓶子
	auto btnBottle = dynamic_cast<ui::Button*>(bg->getChildByName("btnBottle"));
	if (btnBottle)
	{
		btnBottle->setPressedActionEnabled(true);
		btnBottle->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnBottle->setTag(CHAT_BUCKET);
	}

	//公鸡
	auto btnCock = dynamic_cast<ui::Button*>(bg->getChildByName("btnCock"));
	if (btnCock)
	{
		btnCock->setPressedActionEnabled(true);
		btnCock->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnCock->setTag(CHAT_COCK);
	}

	//花
	auto btnFlower = dynamic_cast<ui::Button*>(bg->getChildByName("btnFlower"));
	if (btnFlower)
	{
		btnFlower->setPressedActionEnabled(true);
		btnFlower->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnFlower->setTag(CHAT_FLOWER);
	}

	//番茄
	auto btnTomato = dynamic_cast<ui::Button*>(bg->getChildByName("btnTomato"));
	if (btnTomato)
	{
		btnTomato->setPressedActionEnabled(true);
		btnTomato->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnTomato->setTag(CHAT_TOMATO);
	}

	//握手
	auto btnHandShake = dynamic_cast<ui::Button*>(bg->getChildByName("btnHandShake"));
	if (btnHandShake)
	{
		btnHandShake->setPressedActionEnabled(true);
		btnHandShake->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnHandShake->setTag(CHAT_HANDSHAKE);
	}

	//btnCheers
	auto btnCheers = dynamic_cast<ui::Button*>(bg->getChildByName("btnCheers"));
	if (btnCheers)
	{
		btnCheers->setPressedActionEnabled(true);
		btnCheers->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnCheers->setTag(CHAT_CHEERS);
	}


	//鸡蛋
	auto btnEgg = dynamic_cast<ui::Button*>(bg->getChildByName("btnEgg"));
	if (btnEgg)
	{
		btnEgg->setPressedActionEnabled(true);
		btnEgg->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnEgg->setTag(CHAT_EGG);
	}

	//鞋子
	auto btnShoes = dynamic_cast<ui::Button*>(bg->getChildByName("btnShoes"));
	if (btnShoes)
	{
		btnShoes->setPressedActionEnabled(true);
		btnShoes->addClickEventListener(CC_CALLBACK_1(FindFriendPlayer::onBtnSendExpression, this));
		btnShoes->setTag(CHAT_SHOES);
	}


}

void FindFriendPlayer::onBtnSendExpression(Ref* ref)
{
	if (!m_pSeatNode) return;

	auto btn = dynamic_cast<ui::Button*> (ref);
	if (!btn) return;

	if (m_bSendExpression)
	{
		CommonMethod::showCooldownTips();
		return;
	}

	auto scene = Director::getInstance()->getRunningScene();
	if (!scene)
		return;

	auto nodePlayerInfo = scene->getChildByName("nodePlayerInfo");
	if (!nodePlayerInfo) return;

	auto bg = nodePlayerInfo->getChildByName("playerInfoBg");
	if (!bg) return;

	int iTag = btn->getTag();

	m_pSeatNode->stopActionByTag(TAG_SEND_EXP);
	m_pSeatNode->stopActionByTag(TAG_CONTINUE_EXP);

	m_bSendExpression = true;
	float fOnceTime = 10;

	auto cbTen = dynamic_cast<ui::CheckBox*>(bg->getChildByName("cbTen"));
	if (cbTen->isSelected())
	{
		if (m_gameScene)
			m_gameScene->sendExpression(GetChairID(), iTag);

		Sequence* seqSendExp = Sequence::create(DelayTime::create(fOnceTime), CallFunc::create([=](){
			if (m_gameScene)
				m_gameScene->sendExpression(GetChairID(), iTag);
		}), NULL);
		Repeat* repeat = Repeat::create(seqSendExp, 9);
		repeat->setTag(TAG_CONTINUE_EXP);

		m_pSeatNode->runAction(repeat);
		float fTime = fOnceTime * 10;

		Sequence* seqRecover = Sequence::create(DelayTime::create(fTime), CallFunc::create([=](){
			m_bSendExpression = false;
		}), NULL);

		seqRecover->setTag(TAG_SEND_EXP);
		m_pSeatNode->runAction(seqRecover);
	}
	else
	{
		if (m_gameScene)
			m_gameScene->sendExpression(GetChairID(), iTag);

		Sequence* seqRecover = Sequence::create(DelayTime::create(fOnceTime), CallFunc::create([=](){
			m_bSendExpression = false;
		}), NULL);

		seqRecover->setTag(TAG_SEND_EXP);
		m_pSeatNode->runAction(seqRecover);
	}


	if (nodePlayerInfo)
	{
		Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(nodePlayerInfo);
		nodePlayerInfo->removeFromParent();
	}
}

void FindFriendPlayer::resetHead()
{
	//设置默认头像
	auto avatar = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("sprWeixin"));
	if (avatar)
	{
		auto head = avatar->getChildByName("clipping");
		if (head)
		{
			Sprite* sp = dynamic_cast<Sprite*>(head->getChildByName("head"));
			if (sp)
				sp->setTexture("face/1.png");
		}
		//avatar->setTexture("face/1.png");
		/*avatar->setScale(AVATAR_SIZE / avatar->getContentSize().height);*/
	}

	m_iUserId = 0;
	zeromemory(m_szHeadHttp, sizeof(m_szHeadHttp));
}

void FindFriendPlayer::hideHeadEmpty()
{
	if (m_headEmpty) m_headEmpty->setVisible(false);
}

void FindFriendPlayer::setHeadInfo(const char* szHeadHttp,const char* szIp,int iCount, int iUserId)
{
	zeromemory(m_szHeadHttp, sizeof(m_szHeadHttp));
	memcpy(m_szHeadHttp,szHeadHttp, sizeof(char)* iCount);
	m_iUserId = iUserId;
	m_sIP = szIp;

	m_iGameID = GetGameID();
	m_sNickName = GetNickName();

	auto avatar = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("sprWeixin"));
	if (avatar)
	{
		avatar->setLocalZOrder(0);
		auto roomMaster = m_headEmpty->getChildByName("roomMaster");
		if (roomMaster)
			roomMaster->setLocalZOrder(1);

		downloadHeadUrl(avatar);
		/*if (strcmp(szHeadHttp, "") == 0)
			avatar->setTexture("face/1.png");*/
	//	avatar->setScale(AVATAR_SIZE / avatar->getContentSize().height);
	}

	updatePlayerInfoBox();
}

void FindFriendPlayer::showheadEmpty()
{
	if (m_headEmpty) m_headEmpty->setVisible(true);
}

void FindFriendPlayer::updatePlayerInfoBox()
{
	auto scene = Director::getInstance()->getRunningScene();
	if (!scene)
		return;

	std::string sChildName = "nodePlayerInfo";
	sChildName = sChildName + utility::toString(m_iIdex);
	auto nodePlayerInfo = scene->getChildByName(sChildName);

	if (!nodePlayerInfo) return;

	nodePlayerInfo->setPosition(Director::getInstance()->getWinSize() / 2);

	CommonMethod::initInfoTouchEvent(nodePlayerInfo, "playerInfoBg");

	auto bg = nodePlayerInfo->getChildByName("playerInfoBg");
	if (!bg)
		return;

	auto textName = dynamic_cast<ui::Text*>(bg->getChildByName("textName"));
	if (textName)
		textName->setString(utility::cutString(GetNickName(), 7, ".."));

	auto lScore = GetUserInsure();
	std::string strGold = CommonMethod::converGold(lScore);
	auto textGold = dynamic_cast<ui::Text*>(bg->getChildByName("textGold"));
	if (textGold)
		textGold->setString(strGold);

	auto gender = dynamic_cast<ui::ImageView*>(bg->getChildByName("gender"));
	if (gender)
	{
		gender->setVisible(true);
		gender->ignoreContentAdaptWithSize(true);
		if (m_iGender)
			gender->loadTexture("Common/personInfo/dating_gerenxinxi_touxiangnv_icon.png");
		else
			gender->loadTexture("Common/personInfo/dating_gerenxinxi_touxiangnan_icon.png");
	}

	auto ip = GetLogonIp();
	if (ip == "")
		ip = m_sIP;

	auto textIP = dynamic_cast<ui::Text*>(bg->getChildByName("textIP"));
	if (textIP)
		textIP->setString(ip);

	auto textId = dynamic_cast<ui::Text*>(bg->getChildByName("textId"));
	int iGame = GetGameID();
	if (iGame == 0)
		iGame = m_iGameID;

	if (textId)
		textId->setString(utility::toString(iGame));

	auto btnClose = dynamic_cast<ui::Button*>(bg->getChildByName("btnClose"));
	if (btnClose)
	{
		btnClose->setPressedActionEnabled(true);
		btnClose->addClickEventListener([=](Ref* ref){
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/ui_click.mp3");
			nodePlayerInfo->removeFromParent();
		});
	}

	auto head = bg->getChildByName("head");
	auto headBg = bg->getChildByName("headBg");
	if (head&&headBg)
	{
		headBg->setLocalZOrder(100);

		if (GetUserID() == 0)
		{
			/*if (m_iUserId != 0 && strcmp(m_szHeadHttp, "") != 0)
			ImagicDownManager::Instance().addDown(head, m_szHeadHttp, m_iUserId);*/
			CommonMethod::cutFaceCircle(head, m_iUserId, m_szHeadHttp, 56, "Common/personInfo/dating_gerenxinxi_touxiangwaichen.png");
		}
		else
		{
			/*if (GetHeadHttp() != "")
			ImagicDownManager::Instance().addDown(head, GetHeadHttp(), GetUserID());*/
			CommonMethod::cutFaceCircle(head, GetUserID(), GetHeadHttp(), 56, "Common/personInfo/dating_gerenxinxi_touxiangwaichen.png");
		}

		head->setScale(headBg->getContentSize().height / head->getContentSize().height*0.8);
		head->setPosition(headBg->getPosition());
	}

	initExpressionButton(nodePlayerInfo);
	CommonMethod::initInfoTouchEvent(nodePlayerInfo, "playerInfoBg");
}

bool FindFriendPlayer::isShowHead()
{
	if (!m_headEmpty) return false;

	auto avatar = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("sprWeixin"));
	if (avatar)
		return avatar->isVisible();

	return false;
}

void FindFriendPlayer::hidePlayerInfoBox()
{
	if (!m_pSeatNode) return;

	auto nodePlayerInfo = m_pSeatNode->getChildByName("nodePlayerInfo");
	if (!nodePlayerInfo) return;

	nodePlayerInfo->setVisible(false);
}

void FindFriendPlayer::sePlayerInfo(int iUserId, char szHeadHttp[], int iHeadLen)
{
	m_iUserId = iUserId;
	memcpy(m_szHeadHttp, szHeadHttp, iHeadLen);
}

void FindFriendPlayer::setHeadVisible(bool bVisible)
{
	auto avatar = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("sprWeixin"));
	if (avatar)
		avatar->setVisible(bVisible);
}

int FindFriendPlayer::getUserId()
{
	if (GetUserID() != 0)
		return GetUserID();
	else
		return m_iUserId;
}

void FindFriendPlayer::downloadHeadUrl(Sprite* head)
{
	Sprite* avatar = nullptr;

	if (head == nullptr)
		avatar = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("sprWeixin"));
	else
		avatar = head;

	if (!avatar) return;

	auto headPos = dynamic_cast<ui::ImageView*>(avatar->getChildByName("headPos"));
	if (!headPos) return;
	//headPos->ignoreContentAdaptWithSize(true);
	//headPos->setContentSize(Size(88, 88));

	if (GetUserID() == 0)
	{
		CommonMethod::cutFaceCircle(headPos, m_iUserId, m_szHeadHttp, 43, "findFriend_game/head/touxiangkuang.png");
		//CommonMethod::cutFaceCircleRect(avatar, m_iUserId,m_szHeadHttp, 16, 50);
	}
	else
	{
		CommonMethod::cutFaceCircle(headPos, GetUserID(), GetHeadHttp(), 43, "findFriend_game/head/touxiangkuang.png");
		//CommonMethod::cutFaceCircleRect(avatar, GetUserID(), GetHeadHttp(), 16, 50);
	}
}

void FindFriendPlayer::showGameScore(LONGLONG lScore)
{
	if (!m_headEmpty) return;
	auto numBg = dynamic_cast<Sprite*>(m_headEmpty->getChildByName("numBg"));
	if (!numBg) return;

	numBg->setVisible(true);
	auto number = dynamic_cast<ui::TextAtlas*>(numBg->getChildByName("number"));
	if (!number) return;


	std::string strScore = "";

	int iWidht = 20;
	int iHeight = 24;
	std::string charMapFile = "";

	if (lScore >= 0)
	{
		numBg->setTexture("number/shuzi_jia_faguang.png");
		charMapFile = "number/shuzi_jia.png";
		CommonMethod::setAtlasSymbolAtLast(number, lScore, charMapFile);
	}
	else
	{
		
		numBg->setTexture("number/shuzi_jian_faguang.png");
		charMapFile = "number/shuzi_jian.png";
	}
	CommonMethod::setAtlasSymbolAtLast(number, lScore, charMapFile);

}

void FindFriendPlayer::hideGameScore()
{
	if (!m_headEmpty) return;
	auto numBg = m_headEmpty->getChildByName("numBg");
	if (!numBg) return;

	numBg->setVisible(false);

	auto xi = dynamic_cast<ui::Text*>(m_headEmpty->getChildByName("xi"));
	if (xi)
		xi->setString("");
}

void FindFriendPlayer::cleanVoiceAction()
{
	m_voiceTime = 0.0f;
	m_bVoiceAction = false;
	if (m_voiceActBg)
	{
		m_voiceActBg->stopAllActions();
		m_voiceActBg->setVisible(false);
	}
}

void FindFriendPlayer::showOutCard(BYTE bCard[], int iCount)
{
	hidePass();
	if (!m_pSeatNode) return;

	auto outCardLayout = dynamic_cast<ui::Layout*>(m_pSeatNode->getChildByName("outCardLayout"));
	if (!outCardLayout) return;
	outCardLayout->removeAllChildren();

	outCardLayout->setVisible(true);

	std::string sDir = "findFriend_game/card/";
	std::string sPath;

	sPath = sDir + CFindFriendLogic::Instance().getCardNameByType(0x01);

	FindFriendCard* card = FindFriendCard::create(0x01);

	Size cardSize = card->getContentSize()*CARD_SCALE;
	float offset = cardSize.width / 2;
	float fLeft;
	Size layoutSize = outCardLayout->getContentSize();

	if (iCount >= MAX_OUT_CARD_LINE)
		fLeft = layoutSize.width / 2 - MAX_OUT_CARD_LINE / 2 * offset;
	else
		fLeft = layoutSize.width / 2 - iCount / 2 * offset;

	Point orgPos = Point(fLeft, 0);


	for (int i = 0; i < iCount; i++)
	{
		sPath = sDir + CFindFriendLogic::Instance().getCardNameByType(bCard[i]);
		auto card = FindFriendCard::create(bCard[i]);
		card->setScale(CARD_SCALE);
		card->openNoAction();
		card->setAnchorPoint(Point(.5, 0));
		Point pos = orgPos + Point(offset*i, 0.5*cardSize.height);

		if (i >= MAX_OUT_CARD_LINE)
			pos = orgPos + Point((i%MAX_OUT_CARD_LINE)*offset, 0);

		if (card)
		{
			outCardLayout->addChild(card);
			card->setPosition(pos);
			/*if ((i == iCount - 1)&&m_bBanker)
				card->setCardFlag(FindFriendCard::CARD_FLAG_CALL_FRIEND);*/
		}
	}
}

void FindFriendPlayer::hideLeftCount()
{
	if (!m_pSeatNode) return;

	auto leftCard = m_pSeatNode->getChildByName("leftCard");
	if (leftCard)leftCard->setVisible(false);
}

void FindFriendPlayer::showLeftCardCount(int iCount)
{
	if (!m_pSeatNode) return;

	auto leftCard = m_pSeatNode->getChildByName("leftCard");
	if (!leftCard) return;
	m_iLeftCardCount = iCount;
	/*if (m_iLeftCardCount > 10){
		leftCard->setVisible(false);
		return;
		}*/
	if (m_iLeftCardCount <= 0){
		leftCard->setVisible(false);
		return;
	}
	leftCard->setVisible(true);
	auto cardWarn = leftCard->getChildByName("card_warn");
	if (cardWarn){
		cardWarn->setVisible(false);
		//cardWarn->runAction(CCSequence::create(CCShow::create(), CCBlink::create(1, 4), CCHide::create(), NULL));
	}
	auto textCount = dynamic_cast<ui::TextAtlas*>(leftCard->getChildByName("textCount"));
	if (textCount)
		textCount->setString(utility::toString(iCount));
}

void FindFriendPlayer::hideOutCardLayer()
{
	if (!m_pSeatNode) return;

	auto outCardLayout = dynamic_cast<ui::Layout*>(m_pSeatNode->getChildByName("outCardLayout"));
	if (!outCardLayout) return;
	outCardLayout->removeAllChildren();
	outCardLayout->setVisible(false);
}

void FindFriendPlayer::showPass()
{
	if (!m_pSeatNode) return;

	auto outCardLayout = dynamic_cast<ui::Layout*>(m_pSeatNode->getChildByName("outCardLayout"));
	if (!outCardLayout) return;
	outCardLayout->removeAllChildren();

	cleanOutCardAction();

	auto pass = m_pSeatNode->getChildByName("pass");
	if (pass){
		pass->setVisible(true);
		pass->setScale(0.8);
		pass->runAction(CCScaleTo::create(0.3, 1));
	}
		
}

void FindFriendPlayer::hidePass()
{
	if (!m_pSeatNode) return;
	auto pass = m_pSeatNode->getChildByName("pass");
	if (pass)
		pass->setVisible(false);
}

void FindFriendPlayer::hideOutCardInfo()
{
	if (!m_pSeatNode) return;

	auto outCardLayout = dynamic_cast<ui::Layout*>(m_pSeatNode->getChildByName("outCardLayout"));
	if (!outCardLayout) return;
	outCardLayout->setVisible(false);

	cleanOutCardAction();
	hidePass();
}

void FindFriendPlayer::showOutCardAction(BYTE bCardData[],int iCount)
{
	if (!m_pSeatNode) return;

	auto outCardLayout = dynamic_cast<ui::Layout*>(m_pSeatNode->getChildByName("outCardLayout"));
	if (!outCardLayout) return;

	cleanOutCardAction();
	
	FindFriendCard* card = FindFriendCard::create(0x01, 0x01);

	Size cardSize = card->getContentSize()*CARD_SCALE;
	float offset = cardSize.width / 2;
	Size layoutSize = outCardLayout->getContentSize();

	float fLeft;
	if (iCount >= MAX_OUT_CARD_LINE)
		fLeft = layoutSize.width / 2 - MAX_OUT_CARD_LINE / 2 * offset;
	else
		fLeft = layoutSize.width / 2 - iCount / 2 * offset;

	Point orgPos = Point(fLeft, 0);

	Point pos = Point(orgPos.x+offset*(iCount-1)  / 2, cardSize.height);
	int iCardType = CFindFriendLogic::Instance().GetCardType(bCardData, iCount);

	if (iCardType == CT_SINGLE || iCardType == CT_DOUBLE || iCardType == CT_SAN_ZHANG || iCardType == CT_INVALID) return;

	std::string sTextName;
	std::string sTypeName;

	if (iCardType == CT_SAN_DAI_YI_DUI)
	{
		//三带一对
		sTypeName = "type_1.png";
	}
	else if (iCardType == CT_SHUN_ZI)
	{
		//顺子
		sTypeName = "type_3.png";
	}
	else if (iCardType == CT_TONG_HUA_SHUN)
	{
		//同花顺
		sTypeName = "type_4.png";
	}

	if (sTypeName == "") return;
	//显示文字
	auto spText = Sprite::createWithSpriteFrameName(sTypeName);
	if (spText)
	{
		outCardLayout->addChild(spText);
		spText->setName("outCardText");
		float fScale = 0;
		if (iCardType == CT_TONG_HUA_SHUN)
			fScale = 0.5f;
		else
			fScale = 0.8f;
		spText->setScale(fScale);
		spText->setOpacity(0xff);
		spText->setPosition(pos);
		FadeTo* fadeTo = FadeTo::create(2.0f, 0);
		Sequence* seq = Sequence::create(DelayTime::create(1.0f),fadeTo, RemoveSelf::create(), NULL);
		if (seq)
			spText->runAction(seq);
	}
	
	//显示动画效果
	auto sp = Sprite::create();
	if (sp)
	{
		sp->setName("outCardAction");
		outCardLayout->addChild(sp);
		sp->setPosition(pos);

		Animation* typeAnimation = nullptr; 
		if (iCardType == CT_TONG_HUA_SHUN)
		{
			typeAnimation = AnimationCache::getInstance()->getAnimation("anTongHuaShun");
		}
		else
			typeAnimation = AnimationCache::getInstance()->getAnimation("anShunzi");

		if (typeAnimation)
		{
			auto animate = Animate::create(typeAnimation);
			Sequence* seq = Sequence::create(animate, RemoveSelf::create(), NULL);
			if (seq)
				sp->runAction(seq);
		}
	}
}

void FindFriendPlayer::showTotalScore(LONGLONG lScore)
{
	if (!m_headEmpty) return;

	auto goldBg = m_headEmpty->getChildByName("goldBg");
	if (!goldBg) return;


	auto score = dynamic_cast<ui::Text*>(goldBg->getChildByName("score"));
	if (!score) return;

	if (lScore > 0){
		score->setString("+" + utility::toString(lScore));
	}
	else{
		score->setString(utility::toString(lScore));
	}
}

void FindFriendPlayer::setTotalScoreVisible(bool bVisible)
{
	if (!m_headEmpty) return;

	auto goldBg = m_headEmpty->getChildByName("goldBg");
	if (!goldBg) return;

	goldBg->setVisible(bVisible);
	

}

void FindFriendPlayer::cleanOutCardAction()
{
	if (!m_pSeatNode) return;

	auto outCardLayout = dynamic_cast<ui::Layout*>(m_pSeatNode->getChildByName("outCardLayout"));
	if (!outCardLayout) return;

	auto zhadantexiao = outCardLayout->getChildByName("zhadantexiao");
	if (zhadantexiao)
	{
		zhadantexiao->stopAllActions();
		zhadantexiao->removeFromParent();
	}

	auto outCardText = outCardLayout->getChildByName("outCardText");
	if (outCardText)
	{
		outCardText->stopAllActions();
		outCardText->removeFromParent();
	}

	auto outCardAction = outCardLayout->getChildByName("outCardAction");
	if (outCardAction)
	{
		outCardAction->stopAllActions();
		outCardAction->removeFromParent();
	}
}

void FindFriendPlayer::playTypeSound(BYTE bOutCard[], int iCount)
{
	if (bOutCard[0] == 0 || iCount ==0) return;
	std::string sFileName;
	int iCardType = 0;
	//0男 1女
	if (m_iGender == 0)
	{
		sFileName = "sound/man/";
	}
	else if (m_iGender == 1)
	{
		sFileName = "sound/women/";
	}
	else
		sFileName = "sound/women/";

	iCardType = CFindFriendLogic::Instance().GetCardType(bOutCard, iCount);

	/*
#define CT_SINGLE							1								//单牌 K
#define CT_DOUBLE							2								//对子 22
#define CT_SAN_ZHANG						3								//三张 333
#define CT_SHUN_ZI							4								//顺子
#define CT_TONG_HUA							5								//同花
#define CT_SAN_DAI_YI_DUI					6								//三带一对 444 33
#define CT_FOUR_ONE							7								//四带一
#define CT_TONG_HUA_SHUN					8								//同花顺
	*/
	switch (iCardType)
	{
	case CT_SINGLE:
		{
			int iCardValue = CFindFriendLogic::Instance().GetCardValue(bOutCard[0]);
			if (iCardValue == 1)
				iCardValue = 14;

			sFileName += utility::toString(iCardValue) + ".mp3";
		}
		break;
	case CT_DOUBLE:
		{
					  int iCardValue = CFindFriendLogic::Instance().GetCardValue(bOutCard[0]);
					  if (iCardValue == 1)
						  iCardValue = 14;

					  sFileName += utility::toString(iCardValue) + utility::toString(iCardValue) + ".mp3";
		}
		break;
	case CT_SAN_ZHANG:
		{
					  sFileName += "sanzhang.mp3";
		}
		break;
	case CT_SHUN_ZI:
		{
					  sFileName += "shunzi.mp3";
		}
		break;
	case CT_TONG_HUA:
		{
					sFileName += "tonghua.mp3";
		}
		break;
	case CT_SAN_DAI_YI_DUI:
		{
						sFileName += "3dai2.mp3";
		}
		break;
	case CT_FOUR_ONE:
		{
						sFileName += "4dai1.mp3";
		}
		break;
	case CT_TONG_HUA_SHUN:
		{
							 sFileName += "tonghuashun.mp3";
		}
		break;
	}

	//单张扑克
	//if (iCount == 1)
	//{
	//	int iCardValue = GuanDan_GameLogic::Instance().GetCardValue(bOutCard[0]);
	//	switch (iCardValue)
	//	{
	//	case 1:
	//		iCardValue = 14;
	//		break;
	//	case 0xe:
	//		iCardValue = 15;
	//		break;
	//	case 0xf:
	//		iCardValue = 16;
	//		break;
	//	}

	//	sFileName = sFileName + utility::toString(iCardValue) + ".mp3";
	//}
	//else
	//{
	//	iCardType = GuanDan_GameLogic::Instance().GetCardType(bOutCard, iCount);
	//	if (iCardType == CT_INVALID) return;

	//	if (iCardType == CT_DOUBLE)
	//	{
	//		int iCardValue = 0;
	//		if (GuanDan_GameLogic::Instance().IsLaiZiCard(bOutCard[0]) && GuanDan_GameLogic::Instance().IsLaiZiCard(bOutCard[1])){
	//			iCardValue = GuanDan_GameLogic::Instance().GetCardValue(bOutCard[0]);
	//		}
	//		else if (GuanDan_GameLogic::Instance().IsLaiZiCard(bOutCard[0])){
	//			iCardValue = GuanDan_GameLogic::Instance().GetCardValue(bOutCard[1]);
	//		}
	//		else{
	//			iCardValue = GuanDan_GameLogic::Instance().GetCardValue(bOutCard[0]);
	//		}
	//		
	//		switch (iCardValue)
	//		{
	//		case 1:
	//			iCardValue = 14;
	//			break;
	//		case 0xe:
	//			iCardValue = 15;
	//			break;
	//		case 0xf:
	//			iCardValue = 16;
	//			break;
	//		}

	//		sFileName = sFileName + utility::toString(iCardValue) + utility::toString(iCardValue) + ".mp3";
	//	}
	//	else if (iCardType == CT_SAN_ZHANG)
	//	{
	//		//三张
	//		sFileName = sFileName +"sanzhang" + ".mp3";
	//	}
	//	else if (iCardType == CT_SAN_DAI_YI_DUI)
	//	{
	//		//三带一对
	//		sFileName = sFileName + "three_two" + ".mp3";
	//	}
	//	else if (iCardType == CT_SAN_LIAN_DUI)
	//	{
	//		//连对
	//		sFileName = sFileName + "duiduishun" + ".mp3";
	//	}
	//	else if (iCardType == CT_GANG_BAN)
	//	{
	//		//钢板
	//		sFileName = sFileName + "gangban" + ".mp3";
	//	}
	//	else if (iCardType == CT_SHUN_ZI)
	//	{
	//		sFileName = sFileName + "shunzi" + ".mp3";
	//	}
	//	else if (iCardType == CT_TONG_HUA_SHUN)
	//	{
	//		sFileName = sFileName + "tonghuashun" + ".mp3";
	//	}
	//	else if (iCardType == CT_ZHA_DAN)
	//	{
	//		sFileName = sFileName + "bomb1" + ".mp3";
	//	}
	//	else if (iCardType == CT_ROCKET)
	//	{
	//		sFileName = sFileName + "siwangzha" + ".mp3";
	//	}
	//}

	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(sFileName.c_str());
}

void FindFriendPlayer::playLeftOneSound()
{
	std::string sFileName;
	int iCardType = 0;
	//0男 1女
	if (m_iGender == 0)
	{
		sFileName = "sound/man/";
	}
	else if (m_iGender == 1)
	{
		sFileName = "sound/women/";
	}
	else
		sFileName = "sound/women/";

	sFileName += "leftOneCard.mp3";

	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(sFileName.c_str());
}

void FindFriendPlayer::showXiScore(int iScore)
{
	if (!m_headEmpty) return;

	auto xi = dynamic_cast<ui::Text*>(m_headEmpty->getChildByName("xi"));
	if (xi)
	{
		xi->setVisible(true);
		std::string sContent;
		if (iScore != 0)
		{
			sContent = CommonMethod::getChineseContent("wordXi");
			sContent = sContent + ":" + utility::toString(iScore);
		}
	
		xi->setString(sContent);
	}
		
}

Point FindFriendPlayer::getTimerWorldPos()
{
	Point pos = Point(0, 0);

	if (!m_pSeatNode) return pos;

	Node* node = nullptr;
	//下面的玩家显示在操作按钮旁边
	if (m_iIdex == 0 || m_iIdex == 2)
		node = m_pSeatNode->getChildByName("nodeTimper");
	else
		node = m_pSeatNode->getChildByName("pass");
	
	if (node)
		pos = m_pSeatNode->convertToWorldSpace(node->getPosition());
	
	return pos;
}

void FindFriendPlayer::showWinnerInfo(int iGrade)
{
	//1~4 代表头游 二游 三游 末游，0 表示当前为第一次打牌，不需要显示
	if (!m_headEmpty) return;

	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return;
	
	std::string sFrameName;

	headFront->removeChildByName("spGrade");

	if (iGrade == 0)
	{
		return;
	}
	else if (iGrade == 1)
	{
		sFrameName = "findFriend_game/rank/diyi.png";
		//sFrameName = "rank_1_small.png";
	}
	else if (iGrade == 2)
	{
		sFrameName = "findFriend_game/rank/dier.png";
	}
	else if (iGrade == 3)
	{
		sFrameName = "findFriend_game/rank/disan.png";
	}
	else if (iGrade == 4)
	{
		sFrameName = "findFriend_game/rank/disi.png";
	}

	if (sFrameName == "") return;
	auto spGrade = Sprite::create(sFrameName);
	if (spGrade)
	{
		spGrade->setZOrder(101);
		spGrade->setName("spGrade");
		headFront->addChild(spGrade);
		spGrade->setVisible(true);
		headFront->setVisible(true);
		Point pos;
		//右下角
		pos.x = headFront->getContentSize().width - spGrade->getContentSize().width / 2 - 5;
		pos.y = spGrade->getContentSize().height / 2 + 5;
	//	pos = headFront->getContentSize() / 2;
		spGrade->setPosition(pos);
	}
}

void FindFriendPlayer::showTributeCard(BYTE bCard,BYTE bMainCard)
{
	if (!m_headEmpty) return;

	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return;

	headFront->removeChildByName("card");

	FindFriendCard* card = FindFriendCard::create(bCard, bMainCard);
	if (card)
	{
		card->setName("card");
		card->setZOrder(101);
		card->setScale(0.5);
		headFront->addChild(card);
		Point pos;
		pos.x = card->getContentSize().width / 2*card->getScale() + 5;
		pos.y = card->getContentSize().height / 2*card->getScale() +5;
		card->setPosition(pos);
	}
}

void FindFriendPlayer::cleanTributeInfo()
{
	if (!m_headEmpty) return;
	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return;
	headFront->removeChildByName("card");
	headFront->removeChildByName("icon");
}

void FindFriendPlayer::showEgg(int iGrade)
{
	//1~4 代表头游 二游 三游 末游，0 表示当前为第一次打牌，不需要显示
	if (!m_headEmpty) return;
	auto egg = dynamic_cast<ui::ImageView*>(m_headEmpty->getChildByName("egg"));
	if (!egg) return;
	egg->setVisible(true);
	egg->ignoreContentAdaptWithSize(true);

	std::string sFileName;
	if (iGrade == STATUS_NULL)
	{
		hideEgg();
		return;
	}
	else if (iGrade == STATUS_FIRST)
	{
		sFileName = "rank_1_big.png";
		hideEgg();
	}
	else if (iGrade == STATUS_SECOND)
	{
		sFileName = "rank_2_big.png";
		hideEgg();
	}
	else if (iGrade == STATUS_THIRD)
	{
		sFileName = "rank_3_big.png";
		hideEgg();
	}
	else if (iGrade == STATUS_FOURTH)
	{
		sFileName = "rank_4_big.png";
		hideEgg();
	}
	else if (iGrade == STATUS_CALLFRIEND)
	{
		//喊朋友
		sFileName = "findFriend_game/flag/qinghanpai.png";
	}

	egg->loadTexture(sFileName);
}

void FindFriendPlayer::hideEgg()
{
	if (!m_headEmpty) return;
	auto egg = dynamic_cast<ui::ImageView*>(m_headEmpty->getChildByName("egg"));
	if (egg)
		egg->setVisible(false);

}

void FindFriendPlayer::showNotTribute()
{
	if (!m_headEmpty) return;

	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return;

	headFront->removeChildByName("icon");
	if (m_bGrade != 4) return;

	auto icon = Sprite::createWithSpriteFrameName("sign_defend.png");
	if (icon)
	{
		icon->setZOrder(101);
		icon->setName("icon");
		icon->setVisible(true);
		headFront->addChild(icon);
		Point pos;
		pos.x = icon->getContentSize().width / 2 * icon->getScale() + 5;
		pos.y = icon->getContentSize().height / 2 * icon->getScale() + 5;
		icon->setPosition(pos);
		icon->setScale(1.5);
	}
}

void FindFriendPlayer::cleanNotTribute()
{
	if (!m_headEmpty) return;

	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return;

	headFront->removeChildByName("card");
}

Point FindFriendPlayer::getTributeCardWorldPos()
{
	Point pos;

	if (!m_headEmpty) return pos;

	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return pos;

	FindFriendCard* card = FindFriendCard::create(0x01, 0x01);
	if (card)
	{
		card->setScale(0.25f);
		pos.x = card->getContentSize().width / 2 * card->getScale() + 5;
		pos.y = card->getContentSize().height / 2 * card->getScale() + 5;
	}

	pos = headFront->convertToWorldSpace(pos);
	return pos;
}

void FindFriendPlayer::playPassSound()
{
	std::string sDir;
	std::string sPath;

	if (m_iGender == 0)
		sDir = "sound/man/";
	else
		sDir = "sound/women/";

	sPath = sDir + "pass.mp3";
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(sPath.c_str());
}

void FindFriendPlayer::setGrade(BYTE bGrade)
{
	m_bGrade = bGrade;
}

void FindFriendPlayer::updateLeftCardBg(int iStatus)
{
	if (!m_pSeatNode) return;

	auto leftCard = m_pSeatNode->getChildByName("leftCard");
	if (!leftCard) return;

	auto card = dynamic_cast<ui::ImageView*>(leftCard->getChildByName("card"));
	if (card)
	{
		std::string sName = "findFriend_game/leftCard/pai" +utility::toString(iStatus)+".png";
		card->loadTexture(sName);
	}

	auto textCount = dynamic_cast<ui::TextAtlas*>(m_pSeatNode->getChildByName("textCount"));
	if (textCount)
	{
		std::string filemap = "findFriend_game/leftCard/paiNum" + utility::toString(iStatus) + ".png";
		textCount->setProperty("0", filemap, 26, 36, ".");
	}
}

void FindFriendPlayer::setNickName(std::string sName)
{
	if (!m_headEmpty) return;

	auto name = dynamic_cast<ui::Text*>(m_headEmpty->getChildByName("name"));
	if (name)
		name->setString(sName);
}

void FindFriendPlayer::setNickNameVisible(bool bVisible){
	if (!m_headEmpty) return;

	auto name = dynamic_cast<ui::Text*>(m_headEmpty->getChildByName("name"));
	if (name)
		name->setVisible(bVisible);
}

void FindFriendPlayer::showFriendStatus(int iStatus)
{
	//1~4 代表头游 二游 三游 末游，0 表示当前为第一次打牌，不需要显示
	if (!m_headEmpty) return;

	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return;

	std::string sName;

	headFront->removeChildByName("friendStatus");

	if (iStatus == CALL_NULL)
	{
		return;
	}
	else if (iStatus == MING)
	{
		sName = "findFriend_game/flag/iconcmingpai.png";
		//sFrameName = "rank_1_small.png";
	}
	else if (iStatus == AN)
	{
		sName = "findFriend_game/flag/iconcanpai.png";
	}
	else if (iStatus == FRIEND)
	{
		sName = "findFriend_game/flag/iconczhaopengyou.png";
	}
	
	if (sName == "") return;
	auto friendStatus = Sprite::create(sName);
	if (friendStatus)
	{
		friendStatus->setZOrder(101);
		friendStatus->setName("friendStatus");
		headFront->addChild(friendStatus);
		friendStatus->setVisible(true);
		headFront->setVisible(true);
		Point pos;
		//右上角
		pos.x = headFront->getContentSize().width - friendStatus->getContentSize().width / 2;
		pos.y = headFront->getContentSize().height - friendStatus->getContentSize().height / 2;
		//	pos = headFront->getContentSize() / 2;
		friendStatus->setPosition(pos);
	}
}

void FindFriendPlayer::cleanFriendStatus()
{
	if (!m_headEmpty) return;

	auto headFront = m_headEmpty->getChildByName("sprWeixin");
	if (!headFront) return;

	headFront->removeChildByName("friendStatus");
}

void FindFriendPlayer::removeCards(BYTE bCardData[], int iCount)
{
	if (!m_handCardLayout){
		return;
	}
	if (iCount == 0) return;

	for (int i = 0; i < iCount; i++)
	{
		for (auto card : m_vAllCards)
		{
			if (card->getCardType() == bCardData[i])
			{
				m_vAllCards.eraseObject(card);
				removeSortCard(card);
				card->removeFromParent();
				break;
			}
		}
	}
	calculateAndSortCards();
}

void FindFriendPlayer::clearCards(){
	if (!m_handCardLayout){
		return;
	}
	for (auto card : m_vAllCards)
	{
		if (card)
			card->removeFromParent();
	}
	m_vAllCards.clear();
}

void FindFriendPlayer::SortCards()
{
	if (!m_handCardLayout){
		return;
	}
	if (m_vAllCards.empty()){
		return;
	}

	sort(m_vAllCards.begin(), m_vAllCards.end(), [=](Node* node1, Node* node2){
		FindFriendCard* card1 = dynamic_cast<FindFriendCard*>(node1);
		FindFriendCard* card2 = dynamic_cast<FindFriendCard*>(node2);
		BYTE logic_value1 = CFindFriendLogic::Instance().GetLogicValue(card1->getCardType());
		BYTE logic_value2 = CFindFriendLogic::Instance().GetLogicValue(card2->getCardType());
		if (logic_value1 != logic_value2)
		{
			return logic_value1 > logic_value2;
		}
		else
		{
			return ((card1->getCardType() >> 4) % 4) > ((card2->getCardType() >> 4) % 4);
		}
	});
}

void FindFriendPlayer::addCards(BYTE bCardData[], int iCount)
{
	if (!m_handCardLayout){
		return;
	}
	if (iCount == 0) return;

	for (int i = 0; i < iCount; i++)
	{
		if (bCardData[i] == 0)continue;
		//不会重复添加
		if (isHaveCard(bCardData[i]))continue;

		auto card = FindFriendCard::create(bCardData[i]);
		if (!card) continue;
		m_handCardLayout->addChild(card);
		card->setScale(REPLAY_CARD_SCALE);
		card->openNoAction();
		m_vAllCards.pushBack(card);

		/*FindFriendCard* cards[FIND_FRIEND_HAND_CARD_COUNT + 1];
		cards[0] = card;
		addSortCard(cards, 1, true);*/
	}
	calculateAndSortCards();
}

void FindFriendPlayer::createCards(BYTE bCards[], int iCount)
{
	if (!m_handCardLayout){
		return;
	}
	for (auto card : m_vAllCards)
	{
		if (card)
			card->removeFromParent();
	}
	m_vAllCards.clear();

	for (int i = 0; i < iCount; i++)
	{
		FindFriendCard* card = FindFriendCard::create(bCards[i]);

		if (card)
		{
			card->setScale(REPLAY_CARD_SCALE);
			card->openNoAction();
			//card->setVisible(false);
			m_handCardLayout->addChild(card);
			m_vAllCards.pushBack(card);

			FindFriendCard* cards[FIND_FRIEND_HAND_CARD_COUNT + 1];
			//cards[0] = card;
			//addSortCard(cards, 1, true);
		}
	}

	calculateAndSortCards();
}

void FindFriendPlayer::calculateCardsVect()
{
	if (!m_handCardLayout){
		return;
	}
	if (m_vAllCards.empty()) return;

	auto card = m_vAllCards.front();
	Size cardSize = card->getContentSize()*card->getScale();
	int iCardCount = m_vAllCards.size();
	float offeSet = cardSize.width/2;

	float orginX = 0;
	float fLeft = 0;
	//3 1 4 1 
	//iCardc-1
	orginX = fLeft - (iCardCount - 1) / 2 * offeSet;

	float orginY = 0;
	Point orgPos = Point(orginX, orginY);

	Point cardAnPos = Point(0.5, 0);
	if (m_iIdex == 1)
	{
		cardAnPos = Point(0.5,0);
	}
	else if (m_iIdex == 2)
	{
		cardAnPos = Point(0.5, 1);
	}
	else if (m_iIdex == 3)
	{
		cardAnPos = Point(0.5,0);
	}

	for (int i = 0; i < iCardCount; i++)
	{
		auto card = m_vAllCards.at(i);
		if (!card) continue;

		card->setAnchorPoint(cardAnPos);
		Point pos = orgPos + Point(offeSet*i,0);
		card->setDesPos(pos);
	}


	//for (int i = 0; i < iCardCount; i++)
	//{
	//	int autoSize = m_vAutoSortCards.size();
	//	FindFriendCard* card = nullptr;
	//	
	//		for (int j = m_vAutoSortCards.at(i)->size() - 1; j >= 0; j--){
	//			Point pos = orgPos + Point((i%15)*offeSet, j * 15);
	//			card = m_vAutoSortCards.at(i)->at(j);
	//			if (card){
	//				card->setZOrder(i * 100 + (100 - j));
	//				card->setIdxPos(i, j);
	//				card->setDesPos(pos);
	//				auto cardPoint = m_handCardLayout->convertToWorldSpace(pos);
	//				auto rectSize = card->getContentSize()*card->getScale();
	//				cardPoint.x = cardPoint.x - rectSize.width / 2;
	//				cardPoint.y = cardPoint.y - rectSize.height / 2;

	//				if (iCardCount > 15)
	//				{
	//					//不是最后一张牌,宽是一般
	//					if (i != (iCardCount - 1) || i != (15 - 1))
	//						rectSize.width = rectSize.width / 2;
	//				}
	//				else
	//				{
	//					if (i != (iCardCount - 1))
	//						rectSize.width = rectSize.width / 2;
	//				}


	//				if ((i + 15) < iCardCount)
	//				{
	//					cardPoint.y = cardPoint.y + rectSize.height / 2 + rectSize.height / 4;
	//					rectSize.height = rectSize.height / 2;
	//				}

	//				auto cardRect = Rect(cardPoint, rectSize);
	//				card->setRect(cardRect);

	//				card->setOriginPointAndUpPoint(pos, pos + Point(0, cardSize.height / 8));
	//			}
	//		}
	//}
}

//从排列的向量里删除牌
void FindFriendPlayer::removeSortCard(FindFriendCard* card){
	
	/*for (auto itor = m_vAutoSortCards.begin(); itor != m_vAutoSortCards.end(); itor++){
		Vector<FindFriendCard*>* vec = (*itor);
		if (vec->contains(card)){
		vec->eraseObject(card);
		if (vec->size() == 0){
		m_vAutoSortCards.erase(itor);
		delete vec;
		}
		return;
		}
		}*/
}

void FindFriendPlayer::addSortCard(FindFriendCard* card[], int cardCount, bool isAuto){
	/*if (isAuto){
		for (int i = 0; i < cardCount; i++){
		bool isAdd = false;
		for (auto tmp : m_vAutoSortCards){
		FindFriendCard* card1 = tmp->at(0);
		FindFriendCard* card2 = card[i];
		BYTE logic_value1 = CFindFriendLogic::Instance().GetLogicValue(card1->getCardType());
		BYTE logic_value2 = CFindFriendLogic::Instance().GetLogicValue(card2->getCardType());
		if (logic_value1 == logic_value2){
		isAdd = true;
		tmp->pushBack(card2);
		}
		}
		if (!isAdd){
		Vector<FindFriendCard*>* tmp = new Vector<FindFriendCard*>();
		tmp->pushBack(card[i]);
		m_vAutoSortCards.push_back(tmp);
		}
		}
		}*/
}

bool FindFriendPlayer::isHaveCard(BYTE bCardData)
{
	for (auto card : m_vAllCards)
	{
		if (card->getCardType() == bCardData)
			return true;
	}

	return false;
}

void FindFriendPlayer::calculateAndSortCards()
{
	SortCards();
	calculateCardsVect();
	int iZorder = 0;
	for (auto card : m_vAllCards)
	{
		if (card)
		{
			card->setZOrder(iZorder++);
			card->setPosition(card->getDesPos());
			card->setVisible(true);
		}

	}
}





