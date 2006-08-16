

CReadoutModule* findAdc(string name);
CReadoutModule* findScaler(string name);
void addScaler(CReadoutModule*)
void addAdc(CReadoutModule*);
void setResult(string);

ClearConfiguration();
Configure(string filename);
vector<CReadoutModule*> getAdcs();
vector<CReadoutModule*> getScalers();





