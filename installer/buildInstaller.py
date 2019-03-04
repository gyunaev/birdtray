import os
import logging
import platform
import sys

from argparse import ArgumentParser
from time import strftime
from shutil import copy2, which
from subprocess import check_call


def fillTemplates(path, newPath, templateVariables):
    with open(path) as templateFile:
        with open(newPath, 'w') as outputFile:
            for line in templateFile.readlines():
                outputFile.write(line.format(**templateVariables))


def copyPrerequisites(exePath):
    destination = os.path.join('packages', 'com.ulduzsoft.birdtray.main', 'meta')
    os.makedirs(destination, exist_ok=True)
    copy2(os.path.join('..', 'LICENSE'), destination)
    destination = os.path.join('packages', 'com.ulduzsoft.birdtray.main', 'data')
    os.makedirs(destination, exist_ok=True)
    copy2(exePath, destination)


def createDeploy(winDeployPath):
    logging.getLogger('deploy').info('Creating deployment...')
    exePath = os.path.join('packages', 'com.ulduzsoft.birdtray.main', 'data', 'birdtray.exe')
    check_call([winDeployPath, '--release', '--no-plugins', '--no-system-d3d-compiler',
                '--no-quick-import', '--no-webkit2', '--no-angle', '--no-opengl-sw', '--no-patchqt',
                '-no-svg', exePath])


def createInstaller(binaryCreatorPath, version, arch):
    logging.getLogger('installer').info('Creating installer...')
    installerName = 'birdTray-{version}-{platform}-{arch}.exe'.format(
        version=version, platform=platform.system().lower(), arch=arch)
    check_call([binaryCreatorPath, '-c', os.path.join('config', 'config.xml'),
                '-p', os.path.join('packages'), installerName])


def main():
    parser = ArgumentParser(description='Script to create the birdtray executable.')
    parser.add_argument('-v', '--version', required=True, help='The BirdTray version')
    parser.add_argument('--winDeployPath', required=True, help='Path to the windeployqt executable')
    parser.add_argument('--binaryCreatorPath', required=True,
                        help='Path to the binarycreator executable')
    parser.add_argument('--exePath', required=True, help='Path to the BirdTray executable')
    parser.add_argument('--compilerPath', required=False, help='Path to the g++ executable')
    parser.add_argument('-a', '--arch', required=True,
                        help='Architecture of the BirdTray executable')
    parser.add_argument('-d', '--debug', required=True, default=False, action='store_true',
                        help='Display debugging output')
    arguments = parser.parse_args()

    logging.basicConfig(format='[%(levelname)-5s] %(name)-10s: %(message)s',
                        level=logging.DEBUG if arguments.debug else logging.INFO)
    logger = logging.getLogger('main')

    if which('g++.exe') is None:
        if arguments.compilerPath is None:
            logging.error('g++.exe must be in the path.')
            sys.exit(1)
        os.environ['Path'] += os.path.pathsep + os.path.dirname(arguments.compilerPath)

    templateVariables = {
        'version': arguments.version,
        'date': strftime('%Y-%m-%d'),
    }
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith('.template'):
                path = os.path.join(root, file)
                logger.debug('Filling template {path}'.format(path=path))
                fillTemplates(path, path[:-9], templateVariables)

    copyPrerequisites(arguments.exePath)
    createDeploy(arguments.winDeployPath)
    createInstaller(arguments.binaryCreatorPath, arguments.version, arguments.arch)
    sys.exit(0)


if __name__ == '__main__':
    main()
