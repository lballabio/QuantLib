*** QuantLib-old/ql/Indexes/xibormanager.hpp	Fri Oct 11 13:20:34 2002
--- QuantLib/ql/Indexes/xibormanager.hpp	Wed Oct 23 12:37:07 2002
***************
*** 42,52 ****
              // historical fixings
              static void setHistory(const std::string& name,
                  const History&);
!             static const History& getHistory(const std::string& name);
              static bool hasHistory(const std::string& name);
              static std::vector<std::string> histories();
            private:
!             static HistoryMap historyMap_;
          };
  
      }
--- 42,54 ----
              // historical fixings
              static void setHistory(const std::string& name,
                  const History&);
!             static History getHistory(const std::string& name);
              static bool hasHistory(const std::string& name);
              static std::vector<std::string> histories();
            private:
!             static void checkHistoryMap();
!             static bool initialized_;
!             static HistoryMap* historyMap_;
          };
  
      }
