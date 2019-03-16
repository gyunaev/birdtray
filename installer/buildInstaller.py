import os
import logging
import sys

from argparse import ArgumentParser
from shutil import copy2, which
from subprocess import check_call, CalledProcessError


def copyPrerequisites(exePath):
    destination = 'winDeploy'
    os.makedirs(destination, exist_ok=True)
    copy2(exePath, destination)


def createDeploy(winDeployPath):
    logger = logging.getLogger('deploy')
    logger.info('Creating deployment...')
    exePath = os.path.join('winDeploy', 'birdtray.exe')
    try:
        # noinspection SpellCheckingInspection
        check_call([winDeployPath, '--release', '--no-plugins', '--no-system-d3d-compiler',
                    '--no-quick-import', '--no-webkit2', '--no-angle', '--no-opengl-sw',
                    '--no-patchqt', '-no-svg', exePath])
    except CalledProcessError as error:
        logger.fatal('Failed to create the deployment folder: {error}'.format(error=error))
        logger.debug('Deployment folder creation failure', exc_info=True)


def createInstaller(makeNSISPath):
    logger = logging.getLogger('installer')
    logger.info('Creating installer...')
    try:
        check_call([makeNSISPath, 'installer.nsi'])
    except CalledProcessError as error:
        logger.fatal('Failed to create the installer: {error}'.format(error=error))
        logger.debug('Installer creation failure', exc_info=True)


def main():
    parser = ArgumentParser(description='Script to create the birdtray executable.')
    parser.add_argument('--winDeployPath', required=True, help='Path to the windeployqt executable')
    parser.add_argument('--makeNSISPath', required=True, help='Path to the makensis executable')
    parser.add_argument('--exePath', required=True, help='Path to the BirdTray executable')
    parser.add_argument('--compilerPath', required=False, help='Path to the g++ executable')
    parser.add_argument('-d', '--debug', required=True, default=False, action='store_true',
                        help='Display debugging output')
    arguments = parser.parse_args()

    # noinspection SpellCheckingInspection
    logging.basicConfig(format='[%(levelname)-5s] %(name)-10s: %(message)s',
                        level=logging.DEBUG if arguments.debug else logging.INFO)

    if which('g++.exe') is None:
        if arguments.compilerPath is None:
            logging.error('g++.exe must be in the path.')
            sys.exit(1)
        os.environ['Path'] += os.path.pathsep + os.path.dirname(arguments.compilerPath)

    copyPrerequisites(arguments.exePath)
    createDeploy(arguments.winDeployPath)
    createInstaller(arguments.makeNSISPath)
    sys.exit(0)


if __name__ == '__main__':
    main()
