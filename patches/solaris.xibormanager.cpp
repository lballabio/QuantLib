*** QuantLib-old/ql/Indexes/xibormanager.cpp	Tue Oct 22 13:52:08 2002
--- QuantLib/ql/Indexes/xibormanager.cpp	Wed Oct 23 12:37:32 2002
***************
*** 30,62 ****
  
      namespace Indexes {
  
!         XiborManager::HistoryMap XiborManager::historyMap_;
  
          void XiborManager::setHistory(const std::string& name,
              const History& history) {
!                 historyMap_[name] = history;
          }
  
!         const History& XiborManager::getHistory(const std::string& name) {
!             XiborManager::HistoryMap::const_iterator i =
!                 historyMap_.find(name);
!             QL_REQUIRE(i != historyMap_.end(),
!                 name + " history not loaded");
!             return i->second;
          }
  
          bool XiborManager::hasHistory(const std::string& name) {
!             return (historyMap_.find(name) != historyMap_.end());
          }
  
          std::vector<std::string> XiborManager::histories() {
              std::vector<std::string> temp;
              XiborManager::HistoryMap::const_iterator i;
!             for (i = historyMap_.begin(); i != historyMap_.end(); i++)
                  temp.push_back(i->first);
              return temp;
          }
  
      }
  
  }
--- 30,70 ----
  
      namespace Indexes {
  
!         bool XiborManager::initialized_ = false;
!         XiborManager::HistoryMap* XiborManager::historyMap_ = 0;
  
          void XiborManager::setHistory(const std::string& name,
              const History& history) {
!                 checkHistoryMap();
!                 (*historyMap_)[name] = history;
          }
  
!         History XiborManager::getHistory(const std::string& name) {
!             checkHistoryMap();
! 	    return (*historyMap_)[name];
          }
  
          bool XiborManager::hasHistory(const std::string& name) {
!             checkHistoryMap();
!             return (historyMap_->find(name) != historyMap_->end());
          }
  
          std::vector<std::string> XiborManager::histories() {
+             checkHistoryMap();
              std::vector<std::string> temp;
              XiborManager::HistoryMap::const_iterator i;
!             for (i = historyMap_->begin(); i != historyMap_->end(); i++)
                  temp.push_back(i->first);
              return temp;
          }
  
+         void XiborManager::checkHistoryMap() {
+             if (!initialized_) {
+                 historyMap_ = new HistoryMap;
+                 initialized_ = true;
+             }
+         }
+                 
      }
  
  }
