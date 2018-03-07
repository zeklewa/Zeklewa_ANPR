#include <iostream>
#include <string>
#include <curl/curl.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include "Onvif.h"
#include "xmlParser.h"
#include "IOData.h"
#include "PlateRecognizator.h"
using namespace std;
string UserCameraAddress = IOData::GetCongfigData("UserCameraAddress:").c_str();
string PassCameraAddress = IOData::GetCongfigData("PassCameraAddress:").c_str();
static size_t CallbackFunc(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    size_t oldLength = s->size();
    try
    {
        s->resize(oldLength + newLength);
    }
    catch (std::bad_alloc &e)
    {
        return 0;
    }

    std::copy((char *)contents, (char *)contents + newLength, s->begin() + oldLength);
    return size * nmemb;
}

string intToString(int number)
{
    stringstream ss;
    ss << number;
    return ss.str();
}

string OnvifController::getToken()
{
    std::string respondContent = "";
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, camUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallbackFunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respondContent);


        string strGetProfile = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\"><s:Header><Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>"+UserCameraAddress+"</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"+PassCameraAddress+"</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\"></Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\"></Created></UsernameToken></Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><GetProfiles xmlns=\"http://www.onvif.org/ver10/media/wsdl\"/></s:Body> </s:Envelope>";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strGetProfile.c_str());
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        std::cout << "Returned:" << respondContent;
        return respondContent;
    }
    return "";
}

string OnvifController::post(string content)
{
    std::string respondContent = "";
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, camUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallbackFunc);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respondContent);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return respondContent;
}

int OnvifController::getBrightness()
{
    string imgSettings = "<GetImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\"><VideoSourceToken  xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\">VideoSource0</VideoSourceToken></GetImagingSettings>";
    string cmd = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\"><s:Header><Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>"+UserCameraAddress+"</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"+PassCameraAddress+"</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">" + cameraToken + "</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\"></Created></UsernameToken></Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">" + imgSettings + "</s:Body></s:Envelope>";
    const char* respond = post(cmd).c_str();
    XMLNode xMainNode;
    XMLResults xe;
    xMainNode=XMLNode::parseString(respond,NULL,&xe);
    string bri = xMainNode.getChildNodeByPath("SOAP-ENV:Envelope/SOAP-ENV:Body/timg:GetImagingSettingsResponse/timg:ImagingSettings/tt:Brightness").getText();
    //cout << "resond" << respond << endl;
    return std::stoi(bri);
}
// int
int OnvifController::getExposureTime()
{   
   // cout«"starttttttttttttttttttt"«endl;
    string imgSettings = "<GetImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\"><VideoSourceToken  xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\">VideoSource0</VideoSourceToken></GetImagingSettings>";
    string cmd = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\"><s:Header><Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>"+UserCameraAddress+"</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"+PassCameraAddress+"</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">" + cameraToken + "</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\"></Created></UsernameToken></Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">" + imgSettings + "</s:Body></s:Envelope>";
    const char* respond;
    int cnt = 5;    
    while (cnt > 0) {
        respond = post(cmd).c_str();
        cout << "respond" << respond << endl;
        if (string(respond).find("ExposureTime") != string::npos) {
            //cout<< "empty" << "----" << respond « endl;
            XMLNode xMainNode;
            XMLResults xe;
            xMainNode=XMLNode::parseString(respond,NULL,&xe);
            string ExposureTime = xMainNode.getChildNodeByPath("SOAP-ENV:Envelope/SOAP-ENV:Body/timg:GetImagingSettingsResponse/timg:ImagingSettings/tt:Exposure/tt:ExposureTime").getText();
          
            return std::stoi(ExposureTime);
        }        
        cnt--;
        usleep(100);        
    }
    cout << "WARNING: Get_exposure_time. Cannot get the respond from camera "<< endl;
    return -1;
}

void OnvifController::setBrightness(int level, string videoSourceToken)
{
    string dirCmd = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\"><s:Header><Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>"+UserCameraAddress+"</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"+PassCameraAddress+"</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">" + cameraToken + "</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\"></Created></UsernameToken></Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">" + "<SetImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >" + " <VideoSourceToken  xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >" + videoSourceToken + "</VideoSourceToken>" + "<ImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >" + "<Brightness xmlns=\"http://www.onvif.org/ver10/schema\">" + intToString(level) + " </Brightness>" + "</ImagingSettings>" + "</SetImagingSettings> " + "</s:Body>" + "</s:Envelope>";
    post(dirCmd);
}
int OnvifController::getColorSaturation()
{
    string imgSettings = "<GetImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\"><VideoSourceToken  xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\">VideoSource0</VideoSourceToken></GetImagingSettings>";
    string cmd = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\"><s:Header><Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>"+UserCameraAddress+"</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"+PassCameraAddress+"</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">" + cameraToken + "</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\"></Created></UsernameToken></Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">" + imgSettings + "</s:Body></s:Envelope>";
    const char* respond = post(cmd).c_str();
    XMLNode xMainNode;
    XMLResults xe;
    xMainNode=XMLNode::parseString(respond,NULL,&xe);
    string ColorSat = xMainNode.getChildNodeByPath("SOAP-ENV:Envelope/SOAP-ENV:Body/timg:GetImagingSettingsResponse/timg:ImagingSettings/tt:ColorSaturation").getText();
    //cout << "resond" << respond << endl;
    return std::stoi(ColorSat);
}
void OnvifController:: setExposureTime(int level, string videoSourceToken)
{
    string dirCmd = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\"><s:Header><Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>"+UserCameraAddress+"</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"+PassCameraAddress+"</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">" + cameraToken + "</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\"></Created></UsernameToken></Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
    +"<SetImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >"
    +" <VideoSourceToken  xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >" + videoSourceToken+ "</VideoSourceToken>"
    +"<ImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >"
        +"<Exposure xmlns=\"http://www.onvif.org/ver10/schema\">"
            +"<Mode xmlns=\"http://www.onvif.org/ver10/schema\">MANUAL</Mode>"
            +"<Priority xmlns=\"http://www.onvif.org/ver10/schema\">LowNoise</Priority>"
            +"<Window xmlns=\"http://www.onvif.org/ver10/schema\" bottom="+intToString(1)+" top="+intToString(1)+" right="+intToString(1)+" left="+intToString(1)+" />"
            +"<MinExposureTime xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(1)+"</MinExposureTime>"
            +"<MaxExposureTime xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(30000)+"</MaxExposureTime>"
            +"<MinGain xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(1)+"</MinGain>"
            +"<MaxGain xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(1)+"</MaxGain>"
            +"<MinIris xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(1)+"</MinIris>"
            +"<MaxIris xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(1)+"</MaxIris>"
            +"<ExposureTime xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(level)+"</ExposureTime>"
            +"<Gain xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(1)+"</Gain>"
            +"<Iris xmlns=\"http://www.onvif.org/ver10/schema\">"+intToString(1)+"</Iris>"
        +"</Exposure>"
    +"</ImagingSettings>"
    +"<ForcePersistence xmlns=\"http://www.onvif.org/ver10/schema\">true</ForcePersistence>"
    + "</SetImagingSettings> "
    + "</s:Body>"
    +"</s:Envelope>";
    post(dirCmd);
}
void OnvifController::setColorSaturation(int level, string videoSourceToken)
{
    string dirCmd = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\"><s:Header><Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>"+UserCameraAddress+"</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"+PassCameraAddress+"</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">" + cameraToken + "</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\"></Created></UsernameToken></Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">" + "<SetImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >" + " <VideoSourceToken  xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >" + videoSourceToken + "</VideoSourceToken>" + "<ImagingSettings xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\" >" + "<ColorSaturation xmlns=\"http://www.onvif.org/ver10/schema\">" + intToString(level) + "</ColorSaturation>" + "</ImagingSettings>" + "</SetImagingSettings> " + "</s:Body>" + "</s:Envelope>";
    post(dirCmd);
}

