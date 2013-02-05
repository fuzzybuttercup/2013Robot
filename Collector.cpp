#include "Collector.h"
#include "Portnums.h"

QueueItem::QueueItem(int data){
	next = NULL;
	mData = data;
}

QueueItem::~QueueItem(){
	
}

void QueueItem::AddItem(QueueItem* item){
	if(next == NULL){
		next = item;
	}
	else {
		next->AddItem(item);
	}
}


Queue::Queue(){
	list = NULL;
}

void Queue::push(int data){
	QueueItem* newItem;
	
	newItem = new QueueItem(data);
	if(list == NULL){
		list = newItem;
	}
	else{
		list->AddItem(newItem);				
	}
}

void Queue::pop(){
	QueueItem* deathPointer;
	
	if(list != NULL){
		deathPointer = list;
		list = list->next;
		delete(deathPointer);
		}
	}

bool Queue::empty(){
	return(list == NULL);
}

void Queue::dump(){
	while(!empty()){
		pop();
	}
}

int Queue::top(){
	if(list != NULL){
		return list->mData;
	}
	else return 0;
}



Collector::Collector(UINT32 BotFloorOpenSwitch, UINT32  BotFloorCloseSwitch, UINT32  bucketThingy) :Queue()     {
	
	BottomFloorOpenSwitch = new DigitalInput(BotFloorOpenSwitch);
	BottomFloorCloseSwitch = new DigitalInput(BotFloorCloseSwitch);
	bucketStatusSwitch = new DigitalInput(bucketThingy); 
	FloorDrive = new Relay(FloorMotorRelay);
	IrisServoRight = new Servo(IrisServoRightPort);
	IrisServoLeft = new Servo(IrisServoLeftPort);
	lipDrive = new Servo(LipServo);
	IrisTimer = new Timer();
	state = limbo;
}

void Collector::testOpenIris(){
		state = limbo;
		openIris();
	}
	
void Collector::testCloseIris(){
    	state = limbo;
    	closeIris();
    }
    
void Collector::testUnlockLip(){
    	state = limbo;
    	unlockLip();
    }
    
void Collector::testLockLip(){
    	state = limbo;
    	lockLip();
    }
    
void Collector::testOpenFloor(){
    	state = limbo;
    	openFloor();
    	while (!isFloorOpen());
    	shutoffFloor();
    }
    
void Collector::testCloseFloor(){
    	state = limbo;
    	closeFloor();
    	while (!isFloorClose());
    	shutoffFloor();
    }
    
bool Collector::testFloorClosed(){
    	return isFloorClose();
    }
    
bool Collector::testFloorOpened(){
    	return isFloorOpen();
    }
    
bool Collector::testHaveFrisbee(){
    	return isFrisbeeReady();
    }

void Collector::start() {
	if(state == limbo){
		Init();
	}
	unlockLip();
}

void Collector::unlockLip(){
	lipDrive->Set(unlockLipVal);
}
void Collector::lockLip(){
	lipDrive->Set(lockLipVal);
}

void Collector::openFloor(){
	if(!isFloorOpen()){
	FloorDrive->Set(Relay::kForward);
	}
}
void Collector::openIris(){
	IrisServoRight->Set(unlockRight);
	IrisServoLeft->Set(unlockLeft);
	IrisTimer->Start();
}

void Collector::closeIris(){
	IrisServoRight->Set(lockRight);
	IrisServoLeft->Set(lockLeft);
	IrisTimer->Start();
}

void Collector::closeFloor(){
	if(!isFloorClose()){
	FloorDrive->Set(Relay::kReverse);
	}
}

void Collector::shutoffFloor(){
	FloorDrive->Set(Relay::kOff);
}

bool Collector::isFloorClose(){
	return BottomFloorCloseSwitch->Get();	 
}

bool Collector::isFloorOpen(){
	return BottomFloorOpenSwitch->Get();
}

bool Collector::isFrisbeeReady(){
	return bucketStatusSwitch->Get(); 
}

void Collector::dropDisc(){
	if(state == loaded){
		push(stepCloseIris);
		push(stepOpenFloor);
		push(stepCloseFloor);
		push(stepOpenIris);
		push(stepModeEmpty);
		startStep();
		
	}
}

void Collector::Init(){
	push(stepCloseFloor);
	push(stepOpenIris);
	push(stepModeEmpty);
	startStep();
}

void Collector::Disable(){
	shutoffFloor();
	dump();
	state = limbo;		
}

void Collector::startStep(){
	if(!empty()){
		switch(top()){
			case  stepCloseFloor:
					closeFloor();
					state = running;
			break;
			case stepOpenFloor:
					openFloor();
					state = running;
			break;
			case stepCloseIris:
					closeIris();
					state = running;
			break;
			case stepOpenIris:
					openIris();
					state = running;
			break;
			case stepModeEmpty:
					state = isEmpty;
					pop();
			break;
			case stepModeLoaded:
					state = loaded;
					pop();
			break;
		}
	}
}
void Collector::checkStep(){
	switch(top()){
			case  stepCloseFloor:
				if(isFloorClose()){
					shutoffFloor();
					pop();
					startStep();
				}
			break;
			case stepOpenFloor:
				if(isFloorOpen()){
					shutoffFloor();
					pop();
					startStep();
				}
			break;
			case stepCloseIris:
				if(IrisTimer->HasPeriodPassed(IrisTime)){
					pop();
					startStep();
				}
			break;
			case stepOpenIris:
				if(IrisTimer->HasPeriodPassed(IrisTime)){
					pop();
					startStep();
				}
			break;
			case stepModeEmpty:
				state = isEmpty;
				pop();
				startStep();
				
			break;
			case stepModeLoaded:
				state = loaded;
				pop();
				startStep();
				
			break;
			}
}

void Collector::Idle(){
	switch(state){
	case limbo :
		
	break;
	case isEmpty :
		if(isFrisbeeReady()){
			state = loaded;
			
		}
		
	break;
	case loaded :
		if(!isFrisbeeReady()){
			state = limbo;
		}
		
	break;
	case running :
		checkStep();
	break;
	}
}

	
