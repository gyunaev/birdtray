#!/usr/bin/env python3

import os
import re
import sys
from glob import iglob
from html.parser import HTMLParser

from argparse import ArgumentParser, ZERO_OR_MORE, ONE_OR_MORE
from xml.sax import make_parser
from xml.sax.handler import ContentHandler


class GithubActionsLogger:

    def __init__(self, filePath):
        super().__init__()
        self._filePath = filePath

    def log(self, message, severity, line, column):
        print(f'::{severity} file={self._filePath},line={line},col={column + 1}::{message}')

    def warning(self, message, line, column):
        self.log(message, 'warning', line, column)

    def error(self, message, line, column):
        self.log(message, 'error', line, column)


class Logger:
    WARNING_MESSAGES = {
        'unfinished_translation': 'Unfinished translation',
        'unfinished_missing_attr': 'Unfinished translation without \'type="unfinished"\' attribute',
        'unfinished_with_content': 'Translation is marked as unfinished but has content',
        'unfinished_vanished_translation': 'This translation is no longer used',
        'unfinished_unknown_type': 'The translation has an unknown type "{type}"',
        'whitespace_repeating': 'Possible unwanted repeating whitespaces found',
        'whitespace_start_src_diff': 'Source starts with a whitespace but translation does not',
        'whitespace_start_tr_diff': 'Translation starts with a whitespace but source does not',
        'whitespace_end_src_diff': 'Source ends with a whitespace but translation does not',
        'whitespace_end_tr_diff': 'Translation ends with a whitespace but source does not',
        'newline_missing_in_translation': 'The translation is missing at least one newline',
        'newline_start_src_diff': 'Source starts with a newline but translation does not',
        'newline_start_tr_diff': 'Translation starts with a newline but source does not',
        'newline_end_src_diff': 'Source ends with a newline but translation does not',
        'newline_end_tr_diff': 'Translation ends with a newline but source does not',
        'format_specifier_missing': 'Translation is missing the format specifier "{specifier}"',
        'punctuation_end_differ': 'Translation ends with different punctuation than the source: '
                                  'Expected "{punctuation}", got "{actual}"',
        'filter_invalid': 'Invalid filter id specified: "{filterId}"',
        'html_element_unbalanced': 'The translation has a html element "{element}" '
                                   'that is not present in the source',
        'html_element_diff': 'The translation is missing an expected html element: '
                             'Got "{element}", expected "{sourceElement}"',
        'html_attr_missing': 'The html element "{element}" is missing the attribute "{attribute}"',
        'html_attr_diff': 'The attribute {attribute} of the html element "{element}" has an '
                          'unexpected value: Expected "{sourceValue}", got "{value}"',
        'html_attr_unbalanced': 'The "{element}" html element in the translation has an attribute '
                                '"{attribute}" that is not present in the source',
        'indentation_wrong_start': 'The start of the "{name}" element has an unexpected '
                                   'indentation: Expected "{expected}", got "{actual}"',
        'indentation_wrong_end': 'The end of the "{name}" element has an unexpected '
                                 'indentation: Expected "{expected}", got "{actual}"',
    }
    ERROR_MESSAGES = {
        'missing_language_attr': 'Missing "language" attribute',
        'language_attr_mismatch': '"language" attribute does not match filename: '
                                  'Filename does not end in "{language}"',
        'missing_source': '"message" element is missing required "source" child',
        'missing_translation': '"message" element is missing required "translation" child',
        'format_specifier_unbalanced': 'Translation references a format specifier "{specifier}" '
                                       'which does not exist in the source',
        'special_pattern_missing': 'The following text was expected to be present in the '
                                   'translation, but was not found: "{pattern}"',
        'duplicate_element': 'Duplicated element in message: "{name}"',
        'html_invalid': 'The html is not valid: {message}',
    }

    def __init__(self, filePath, globalFilter=None):
        super().__init__()
        self._ghLogger = GithubActionsLogger(filePath)
        self._warningCount = self._errorCount = 0
        self._globalFilter = set() if globalFilter is None else globalFilter
        self._localFilter = set()

    def warning(self, warningId, position, **kwargs):
        if self._isFiltered(warningId):
            return
        self._warningCount += 1
        self._ghLogger.warning(
            self.WARNING_MESSAGES[warningId].format(**kwargs) + f' ({warningId})', *position)

    def error(self, errorId, position, **kwargs):
        if self._isFiltered(errorId):
            return
        self._errorCount += 1
        self._ghLogger.error(
            self.ERROR_MESSAGES[errorId].format(**kwargs) + f' ({errorId})', *position)

    def getNumWarnings(self):
        return self._warningCount

    def getNumErrors(self):
        return self._errorCount

    def addLocalFilter(self, filterId):
        self._localFilter.add(filterId)

    def clearLocalFilters(self):
        self._localFilter.clear()

    def _isFiltered(self, filterId):
        return filterId in self._globalFilter | self._localFilter


class HTMLComparer(HTMLParser):

    def __init__(self, logger, basePosition):
        super().__init__()
        self._logger = logger
        self._basePosition = basePosition
        self._doCompare = False
        self._sourceStartElements = []
        self._sourceEndElements = []

    def error(self, message):
        self._logger.error('html_invalid', self.getPosition(), message=message)

    def handle_starttag(self, tag, attrs):
        if self._doCompare:
            try:
                sourceTag, sourceAttributes = self._sourceStartElements.pop(0)
            except IndexError:
                self._logger.warning('html_element_unbalanced', self.getPosition(), element=tag)
                raise ValueError('Unbalanced html element')
            if sourceTag != tag:
                self._logger.warning('html_element_diff', self.getPosition(),
                                     sourceElement=sourceTag, element=tag)
                raise ValueError('Different html element')
            for name, value in sourceAttributes:
                for i, attribute in enumerate(attrs):
                    if name == attribute[0]:
                        del attrs[i]
                        break
                else:
                    self._logger.warning(
                        'html_attr_missing', self.getPosition(), element=tag, attribute=name)
                    continue
                if value != attribute[1]:
                    self._logger.warning('html_attr_diff', self.getPosition(), element=tag,
                                         attribute=name, value=attribute[1], sourceValue=value)
            for name, _ in attrs:
                self._logger.warning(
                    'html_attr_unbalanced', self.getPosition(), element=tag, attribute=name)

        else:
            self._sourceStartElements.append((tag, attrs))

    def handle_endtag(self, tag):
        if self._doCompare:
            try:
                sourceTag = self._sourceEndElements.pop(0)
            except IndexError:
                self._logger.warning('html_element_unbalanced', self.getPosition(), element=tag)
                raise ValueError('Unbalanced html element')
            if sourceTag != tag:
                self._logger.warning('html_element_diff', self.getPosition(),
                                     sourceElement=sourceTag, element=tag)
                raise ValueError('Different html element')

        else:
            self._sourceEndElements.append(tag)

    def compare(self, html):
        self.reset()
        self._doCompare = True
        try:
            self.feed(html)
        except ValueError:
            pass

    def getPosition(self):
        line, column = self._basePosition
        htmlLine, htmlColumn = self.getpos()
        if htmlLine > 1:
            column = 0
        return line + htmlLine - 1, column + htmlColumn


class TranslationHandler(ContentHandler):
    FILTER_REGEX = re.compile(r'checkTranslation\signore:\s*(?P<filters>.+?)(\s|$)')
    SENTENCE_ENDING_PUNCTUATIONS = '.,!?:;…	¡¿։'

    def __init__(self, filePath, formatSpecifierRegexes, specialPatterns, globalWarningFilter):
        super().__init__()
        self._locator = None
        self._filePath = filePath
        self._logger = Logger(filePath, globalWarningFilter)
        self._level = -1
        self._levelCharacters = ' ' * 4
        self._currentElement = None
        self._lastData = ''
        self._translationAttributes = None
        self._source = None
        self._sourcePos = None
        self._translation = None
        self._translationPos = None
        self._messageChildren = []
        self._formatSpecifierRegexes = formatSpecifierRegexes
        self._specialPatterns = specialPatterns

    def setDocumentLocator(self, locator):
        super().setDocumentLocator(locator)
        self._locator = locator

    def startElement(self, name, attrs):
        self._currentElement = name
        if self._lastData != self._levelCharacters * self._level and \
                (self._level != 0 or self._lastData != '\n'):
            self.warning('indentation_wrong_start', actual=repr(self._lastData)[1:-1],
                         expected=repr(self._levelCharacters * self._level)[1:-1], name=name)
        self._level += 1
        if name == 'message':
            self._source = None
            self._sourcePos = None
            self._translation = None
            self._translationPos = None
            self._translationAttributes = None
            self._messageChildren.clear()
            self._logger.clearLocalFilters()
        elif name == 'source':
            self._source = ''
        elif self._currentElement == 'translation':
            self._translationAttributes = attrs
            self._translation = ''
        elif name == 'TS':
            languageAttribute = attrs.get('language')
            if languageAttribute is None:
                self.error('missing_language_attr')
            else:
                language = languageAttribute[:2].lower()
                if not os.path.splitext(os.path.basename(self._filePath))[0] \
                        .endswith('_' + language):
                    self.error('language_attr_mismatch', language=language)
        if name in self._messageChildren:
            self.error('duplicate_element', name=name)
        else:
            self._messageChildren.append(name)

    def endElement(self, name):
        self._currentElement = None
        self._level -= 1
        if name not in ['source', 'translation', 'translatorcomment'] and \
                self._lastData != self._levelCharacters * self._level and \
                all(char.isspace() for char in self._lastData) and \
                (self._level > 0 or self._lastData != '\n'):
            self.warning('indentation_wrong_end', actual=repr(self._lastData)[1:-1],
                         expected=repr(self._levelCharacters * self._level)[1:-1], name=name)
        if name == 'source':
            if self._sourcePos is None:
                self._sourcePos = self._locator.getLineNumber(), self._locator.getColumnNumber()
            return
        if name == 'translation':
            if self._translationPos is None:
                self._translationPos = \
                    self._locator.getLineNumber(), self._locator.getColumnNumber()
            return
        if name != 'message':
            return
        if self._source is None or self._translation is None:
            if self._source is None:
                self.error('missing_source')
            if self._translation is None:
                self.error('missing_translation')
            return
        isUnfinished = self._checkUnfinished()
        if not isUnfinished:
            self._checkWhitespaces()
            self._checkNewlines()
            self._checkFormatSpecifiers()
            self._checkPunctuation()
            self._checkSpecialPatterns()
            self._checkHtml()

    def characters(self, data):
        self._lastData = data
        if self._currentElement == 'source':
            if self._sourcePos is None:
                self._sourcePos = self._locator.getLineNumber(), self._locator.getColumnNumber()
            self._source += data
        elif self._currentElement == 'translation':
            if self._translationPos is None:
                self._translationPos = \
                    self._locator.getLineNumber(), self._locator.getColumnNumber()
            self._translation += data
        elif self._currentElement == 'translatorcomment':
            filterMatch = self.FILTER_REGEX.search(data)
            if filterMatch:
                index = filterMatch.start('filters')
                for filterId in filterMatch.group('filters').split(','):
                    if filterId in Logger.WARNING_MESSAGES or filterId in Logger.ERROR_MESSAGES:
                        self._logger.addLocalFilter(filterId)
                    else:
                        position = (self._locator.getLineNumber(),
                                    self._locator.getColumnNumber() + index)
                        self.warning('filter_invalid', position, filterId=filterId)
                    index += len(filterId) + 1

    def getNumWarnings(self):
        return self._logger.getNumWarnings()

    def getNumErrors(self):
        return self._logger.getNumErrors()

    def warning(self, warningId, position=None, **kwargs):
        if position is None:
            position = self._locator.getLineNumber(), self._locator.getColumnNumber()
        self._logger.warning(warningId, position, **kwargs)

    def error(self, errorId, position=None, **kwargs):
        if position is None:
            position = self._locator.getLineNumber(), self._locator.getColumnNumber()
        self._logger.error(errorId, position, **kwargs)

    def _checkUnfinished(self):
        translationType = None if self._translationAttributes is None else \
            self._translationAttributes.get('type')
        if translationType == 'unfinished':
            if len(self._translation) != 0:
                self.warning('unfinished_with_content', self._translationPos)
            else:
                self.warning('unfinished_translation', self._translationPos)
            return True
        elif translationType == 'vanished':
            self.warning('unfinished_vanished_translation', self._translationPos)
        else:
            if len(self._translation.strip()) == 0 and len(self._source.strip()) != 0:
                self.warning('unfinished_missing_attr', self._translationPos)
            if translationType is not None:
                self.warning('unfinished_unknown_type', self._translationPos, type=translationType)
        return False

    def _checkWhitespaces(self):
        if '  ' in self._translation and '  ' not in self._source:
            index = 0
            while True:
                try:
                    index = self._translation.index('  ', index + 2)
                except ValueError:
                    break
                self.warning('whitespace_repeating', self._calculatePosition(
                    self._translation, index, self._translationPos))
        if self._source.startswith(' '):
            if not self._translation.startswith(' '):
                self.warning('whitespace_start_src_diff', self._translationPos)
        elif self._translation.startswith(' '):
            self.warning('whitespace_start_tr_diff', self._translationPos)
        if self._source.endswith(' '):
            if not self._translation.endswith(' '):
                self.warning('whitespace_end_src_diff', self._calculatePosition(
                    self._translation, len(self._translation), self._translationPos))
        elif self._translation.endswith(' '):
            self.warning('whitespace_end_tr_diff', self._calculatePosition(
                self._translation, len(self._translation), self._translationPos))

    def _checkNewlines(self):
        reportedMissingNewline = False
        if self._source.startswith('\n'):
            if not self._translation.startswith('\n'):
                reportedMissingNewline = True
                self.warning('newline_start_src_diff', self._translationPos)
        elif self._translation.startswith('\n'):
            self.warning('newline_start_tr_diff', self._translationPos)
        if self._source.endswith('\n'):
            if not self._translation.endswith('\n'):
                reportedMissingNewline = True
                self.warning('newline_end_src_diff', self._calculatePosition(
                    self._translation, len(self._translation), self._translationPos))
        elif self._translation.endswith('\n'):
            self.warning('newline_end_tr_diff', self._calculatePosition(
                self._translation, len(self._translation), self._translationPos))
        if not reportedMissingNewline and self._source.count('\n') > self._translation.count('\n'):
            self.warning('newline_missing_in_translation', self._translationPos)

    def _checkFormatSpecifiers(self):
        for formatSpecifierRegex in self._formatSpecifierRegexes:
            sourceSpecifiers = set(formatSpecifierRegex.findall(self._source))
            translationSpecifiers = set(formatSpecifierRegex.findall(self._translation))
            for specifier in sourceSpecifiers ^ translationSpecifiers:
                if specifier in sourceSpecifiers:
                    self.warning('format_specifier_missing',
                                 self._translationPos, specifier=specifier)
                else:
                    self.error('format_specifier_unbalanced', self._calculatePosition(
                        self._translation, self._translation.index(specifier),
                        self._translationPos), specifier=specifier)

    def _checkPunctuation(self):
        if self._source == '' or self._translation == '':
            return
        lastSourceChar = self._source[-1]
        lastTranslationChar = self._translation[-1]
        if lastSourceChar not in self.SENTENCE_ENDING_PUNCTUATIONS and \
                lastTranslationChar not in self.SENTENCE_ENDING_PUNCTUATIONS:
            return  # If source and translation don't end in a punctuation
        if lastSourceChar != lastTranslationChar:
            if not (lastSourceChar == '.' and self._source.endswith('...') and
                    self._translation.endswith('…')) and \
                    not (lastSourceChar == '…' and self._translation.endswith('...')):
                self.warning('punctuation_end_differ', self._calculatePosition(
                    self._translation, len(self._translation) - 1,
                    self._translationPos), punctuation=lastSourceChar, actual=lastTranslationChar)

    def _checkSpecialPatterns(self):
        for pattern in self._specialPatterns:
            for sourceMatch in pattern.finditer(self._source):
                text = sourceMatch.group('match')
                if text not in self._translation:
                    self.error('special_pattern_missing', self._translationPos,
                               pattern=sourceMatch.group('match'))

    def _checkHtml(self):
        if '<html>' not in self._source or '</html>' not in self._source:
            return
        parser = HTMLComparer(self._logger, self._translationPos)
        parser.feed(self._source)
        parser.compare(self._translation)

    @staticmethod
    def _calculatePosition(string, index, filePos):
        lineNo, column = filePos
        for line in string.split('\n'):
            length = len(line)
            if index <= length:
                return lineNo, column + index
            index -= length + 1
            lineNo += 1
            column = 0
        raise ValueError('Index is not in string')


def processFile(translationFile, formatSpecifierRegexes, specialPatterns, globalWarningFilter):
    print(f'Processing {translationFile}...')
    parser = make_parser()
    handler = TranslationHandler(
        translationFile, formatSpecifierRegexes, specialPatterns, globalWarningFilter)
    parser.setContentHandler(handler)
    parser.parse(translationFile)
    numErrors = handler.getNumErrors()
    print(f'Found {handler.getNumWarnings()} warning(s) and {numErrors} error(s)')
    return numErrors


def main(translationFiles, formatSpecifierRegexes, specialPatterns, warnings):
    globalWarningFilter = set() if warnings is None else set([
        warningId for warningId in warnings.split(',')])
    formatSpecifierRegexes = [re.compile(regex) for regex in formatSpecifierRegexes]
    specialPatterns = [re.compile(regex) for regex in specialPatterns]
    numErrors = 0
    for pathSpec in translationFiles:
        for path in iglob(pathSpec, recursive=True):
            numErrors += processFile(
                path, formatSpecifierRegexes, specialPatterns, globalWarningFilter)
    return 0 if numErrors == 0 else 1


def parseCmd():
    """
    Parse the command line

    :return: The parsed arguments.
    """
    parser = ArgumentParser(description='Check Qt translation files for common problems and errors')
    parser.add_argument('translationFiles', nargs=ONE_OR_MORE,
                        help='The path specification to the Qt translation files to check')
    # noinspection SpellCheckingInspection
    parser.add_argument('-r', '--formatSpecifierRegex', nargs=ZERO_OR_MORE,
                        default=[r'(?<!%)(?P<formatSpecifier>%(?:\d+|[diuoxXfFeEgGaAcspn]))'],
                        help='Regex for format specifiers')
    parser.add_argument('-p', '--specialPatterns', nargs=ZERO_OR_MORE,
                        default=[r'(?P<match>\s\(\*\.\S+(?:\s\*\.\w+)*\))', r'(?P<match>\d+)'],
                        help='Regex for special patterns that must be present '
                             'in the translation if found in the source')
    parser.add_argument('-i', '--ignoreWarning', required=False, default=None,
                        help='Comma separated list of global warning filters')
    return parser.parse_args()


if __name__ == '__main__':
    _args = parseCmd()
    sys.exit(main(_args.translationFiles, _args.formatSpecifierRegex, _args.specialPatterns,
                  _args.ignoreWarning))
