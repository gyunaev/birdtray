#ifndef BIRDTRAY_TEST_RESOURCE_H
#define BIRDTRAY_TEST_RESOURCE_H


#include <QtCore/QString>

/**
 * A class for handling resources for the tests.
 */
class TestResources {
public:
    /**
     * Get the absolute path to a test resource file.
     *
     * @param name The name of the test resource file.
     * @return The absolute path to the test resource.
     */
    static QString getAbsoluteResourcePath(const QString& name);

private:
    
    /**
     * @return The absolute path to the test resource directory.
     */
    static QString getResourceBasePath();
};


#endif /* BIRDTRAY_TEST_RESOURCE_H */
