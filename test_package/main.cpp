#include <iostream>
#include <string>

#include <QString>

#include <KCoreAddons>

static const QString TESTVALUE = "5.61.0";

int main()
{

    QString readValue = KCoreAddons::versionString();

    if (readValue == TESTVALUE)
    {
        std::cout << "Test OK" << std::endl;
        return 0;
    }
    std::cout << "Test FAILED" << std::endl;
    return 1;
}
