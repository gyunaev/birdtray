#!/usr/bin/env python3

import os
import re
import sys
from glob import iglob
from html.parser import HTMLParser

from argparse import ArgumentParser, ZERO_OR_MORE, ONE_OR_MORE
from xml.sax import make_parser, SAXParseException
from xml.sax.handler import ContentHandler


class GithubActionsLogger:
    """ A logger that logs messages in the GitHub Actions format. """

    def __init__(self, filePath):
        """
        :param filePath: The path of the file that the logs belong to.
        """
        super().__init__()
        self._filePath = filePath

    def log(self, message, severity, line, column):
        """
        Log the message with the specified severity.

        :param message: The message to log.
        :param severity: The severity to log.
        :param line: The line number in the file the message belongs to.
        :param column: The column number in the line the message belongs to.
        """
        print(f'::{severity} file={self._filePath},line={line},col={column + 1}::{message}')

    def warning(self, message, line, column):
        """
        Log a warning message.

        :param message: The message to log.
        :param line: The line number in the file the message belongs to.
        :param column: The column number in the line the message belongs to.
        """
        self.log(message, 'warning', line, column)

    def error(self, message, line, column):
        """
        Log an error message.

        :param message: The message to log.
        :param line: The line number in the file the message belongs to.
        :param column: The column number in the line the message belongs to.
        """
        self.log(message, 'error', line, column)


class Logger:
    """ A logger that handles logging warnings and errors. """
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
        'unexpected_text': 'Unexpected text found, check last closing tag: {text}',
    }
    ERROR_MESSAGES = {
        'missing_language_attr': 'Missing "language" attribute',
        'language_attr_mismatch': '"language" attribute does not match filename: '
                                  'Filename does not end in "{language}"',
        'missing_source': '"message" element is missing required "source" child',
        'missing_translation': '"message" element is missing required "translation" child',
        'format_specifier_missing': 'Translation is missing the format specifier "{specifier}"',
        'format_specifier_unbalanced': 'Translation references a format specifier "{specifier}" '
                                       'which does not exist in the source',
        'special_pattern_missing': 'The following text was expected to be present in the '
                                   'translation, but was not found: "{pattern}"',
        'duplicate_element': 'Duplicated element in message: "{name}"',
        'html_invalid': 'The html is not valid: {message}',
        'xml_invalid': 'The xml is not valid: {message}',
    }

    def __init__(self, filePath, globalFilter=None):
        """
        :param filePath: The path of the file that the logs belong to.
        :param globalFilter: A warning filter tat is always active.
        """
        super().__init__()
        self._ghLogger = GithubActionsLogger(filePath)
        self._warningCount = self._errorCount = 0
        self._globalFilter = set() if globalFilter is None else globalFilter
        self._localFilter = set()

    def warning(self, warningId, position, **kwargs):
        """
        Log a warning message.

        :param warningId: The id of the warning.
        :param position: The line and column in the file this warning belongs to.
        :param kwargs: Formatting arguments for the warning message.
        """
        if self._isFiltered(warningId):
            return
        self._warningCount += 1
        self._ghLogger.warning(
            self.WARNING_MESSAGES[warningId].format(**kwargs) + f' ({warningId})', *position)

    def error(self, errorId, position, **kwargs):
        """
        Log an error message.

        :param errorId: The id of the error.
        :param position: The line and column in the file this error belongs to.
        :param kwargs: Formatting arguments for the error message.
        """
        if self._isFiltered(errorId):
            return
        self._errorCount += 1
        self._ghLogger.error(
            self.ERROR_MESSAGES[errorId].format(**kwargs) + f' ({errorId})', *position)

    def getNumWarnings(self):
        """
        :return: The total number of warnings generated.
        """
        return self._warningCount

    def getNumErrors(self):
        """
        :return: The total number of errors messages generated.
        """
        return self._errorCount

    def addLocalFilter(self, filterId):
        """
        Add the specific warning or error id to the local filter.
        warnings in the filter won't be displayed.

        :param filterId: The error or warning id to filter.
        """
        self._localFilter.add(filterId)

    def clearLocalFilters(self):
        """ Clear the local warning and error filter. """
        self._localFilter.clear()

    def _isFiltered(self, filterId):
        """
        :param filterId: The error or warning id.
        :return: Whether or not the id is filtered out.
        """
        return filterId in self._globalFilter | self._localFilter


class HTMLComparer(HTMLParser):
    """ A HTML parser that can compare tow HTML inputs. """

    def __init__(self, logger, basePosition):
        """
        :param logger: The logger to use to display warnings and errors.
        :param basePosition: the start line and column of the html input in the source file.
        """
        super().__init__()
        self._logger = logger
        self._basePosition = basePosition
        self._doCompare = False
        self._sourceStartElements = []
        self._sourceEndElements = []

    def error(self, message):
        """
        Handle a HTML parsing error.

        :param message: The error message.
        """
        self._logger.error('html_invalid', self._getPosition(), message=message)

    def handle_starttag(self, tag, attrs):
        """
        Handle an opening HTML element.

        :param tag: The name of the element.
        :param attrs: The attributes of the element.
        """
        if self._doCompare:
            try:
                sourceTag, sourceAttributes = self._sourceStartElements.pop(0)
            except IndexError:
                self._logger.warning('html_element_unbalanced', self._getPosition(), element=tag)
                raise ValueError('Unbalanced html element')
            if sourceTag != tag:
                self._logger.warning('html_element_diff', self._getPosition(),
                                     sourceElement=sourceTag, element=tag)
                raise ValueError('Different html element')
            for name, value in sourceAttributes:
                for i, attribute in enumerate(attrs):
                    if name == attribute[0]:
                        del attrs[i]
                        break
                else:
                    self._logger.warning(
                        'html_attr_missing', self._getPosition(), element=tag, attribute=name)
                    continue
                if value != attribute[1]:
                    self._logger.warning('html_attr_diff', self._getPosition(), element=tag,
                                         attribute=name, value=attribute[1], sourceValue=value)
            for name, _ in attrs:
                self._logger.warning(
                    'html_attr_unbalanced', self._getPosition(), element=tag, attribute=name)

        else:
            self._sourceStartElements.append((tag, attrs))

    def handle_endtag(self, tag):
        """
        Handle a closing HTML element.

        :param tag: The name of the element.
        """
        if self._doCompare:
            try:
                sourceTag = self._sourceEndElements.pop(0)
            except IndexError:
                self._logger.warning('html_element_unbalanced', self._getPosition(), element=tag)
                raise ValueError('Unbalanced html element')
            if sourceTag != tag:
                self._logger.warning('html_element_diff', self._getPosition(),
                                     sourceElement=sourceTag, element=tag)
                raise ValueError('Different html element')

        else:
            self._sourceEndElements.append(tag)

    def compare(self, html):
        """
        Compare the previously read HTML with the new one.

        :param html: The new HTML string.
        """
        self.reset()
        self._doCompare = True
        try:
            self.feed(html)
        except ValueError:
            pass

    def _getPosition(self):
        """
        :return: The current position of the HTML parser in respect to the translation file.
        """
        line, column = self._basePosition
        htmlLine, htmlColumn = self.getpos()
        if htmlLine > 1:
            column = 0
        return line + htmlLine - 1, column + htmlColumn


class TranslationHandler(ContentHandler):
    """ Linter for a Qt language translation file. """
    FILTER_REGEX = re.compile(r'checkTranslation\signore:\s*(?P<filters>.+?)(\s|$)')
    SENTENCE_ENDING_PUNCTUATIONS = '.,!?:;…	¡¿։'

    def __init__(self, filePath, formatSpecifierRegexes, specialPatterns, globalWarningFilter):
        """
        Initialize the linter.

        :param filePath: The path of the translation file.
        :param formatSpecifierRegexes: A list of regexes for matching format specifiers.
        :param specialPatterns: A list of regexes for matching patterns
                                which must be equal in the source and translation.
        :param globalWarningFilter: A warning and error filter for the whole file.
        """
        super().__init__()
        self._locator = None
        self._filePath = filePath
        self._logger = Logger(filePath, globalWarningFilter)
        self._level = -1
        self._levelCharacters = ' ' * 4
        self._elementStack = [None]
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
        """
        Handle the start of an xml element in the translation file.

        :param name: The name of the element.
        :param attrs: The attributes of the element.
        """
        self._elementStack.append(name)
        if self._lastData != self._levelCharacters * self._level and \
                (self._level != 0 or self._lastData != '\n'):
            self.warning('indentation_wrong_start', actual=self._escapeText(self._lastData),
                         expected=self._escapeText(self._levelCharacters * self._level), name=name)
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
        elif self._currentElement() == 'translation':
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
                    self.error('language_attr_mismatch', language=self._escapeText(language))
        if name in self._messageChildren:
            self.error('duplicate_element', name=self._escapeText(name))
        else:
            self._messageChildren.append(name)

    def endElement(self, name):
        """
        Handle the end of an xml element.

        :param name: The name of the element.
        """
        lastName = self._elementStack.pop()
        assert lastName == name  # The parser should throw an exception and not call endElement.
        self._level -= 1
        if name not in ['source', 'translation', 'translatorcomment'] and \
                self._lastData != self._levelCharacters * self._level and \
                all(char.isspace() for char in self._lastData) and \
                (self._level > 0 or self._lastData != '\n'):
            self.warning('indentation_wrong_end', actual=self._escapeText(self._lastData),
                         expected=self._escapeText(self._levelCharacters * self._level), name=name)
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
        """
        Handle contents of xml elements.

        :param data: The content of the current element.
        """
        self._lastData = data
        if self._currentElement() == 'source':
            if self._sourcePos is None:
                self._sourcePos = self._locator.getLineNumber(), self._locator.getColumnNumber()
            self._source += data
        elif self._currentElement() == 'translation':
            if self._translationPos is None:
                self._translationPos = \
                    self._locator.getLineNumber(), self._locator.getColumnNumber()
            self._translation += data
        elif self._currentElement() == 'translatorcomment':
            filterMatch = self.FILTER_REGEX.search(data)
            if filterMatch:
                index = filterMatch.start('filters')
                for filterId in filterMatch.group('filters').split(','):
                    if filterId in Logger.WARNING_MESSAGES or filterId in Logger.ERROR_MESSAGES:
                        self._logger.addLocalFilter(filterId)
                    else:
                        position = (self._locator.getLineNumber(),
                                    self._locator.getColumnNumber() + index)
                        self.warning(
                            'filter_invalid', position, filterId=self._escapeText(filterId))
                    index += len(filterId) + 1
        elif self._currentElement() in ['message', 'context', 'TS'] \
                and not data.isspace() and data != '':
            self.warning('unexpected_text', text=self._escapeText(data))

    def getNumWarnings(self):
        """
        :return: The total amount of warnings emitted so far.
        """
        return self._logger.getNumWarnings()

    def getNumErrors(self):
        """
        :return: The total amount of errors emitted so far.
        """
        return self._logger.getNumErrors()

    def warning(self, warningId, position=None, **kwargs):
        """
        Emit a warning.

        :param warningId: The id of the warning.
        :param position: The position of the warning. Defaults to the current parser position.
        :param kwargs: Formatting arguments for the warning message.
        """
        if position is None:
            position = self._locator.getLineNumber(), self._locator.getColumnNumber()
        self._logger.warning(warningId, position, **kwargs)

    def error(self, errorId, position=None, **kwargs):
        """
        Emit an error.

        :param errorId: The id of the error.
        :param position: The position of the error. Defaults to the current parser position.
        :param kwargs: Formatting arguments for the error message.
        """
        if position is None:
            position = self._locator.getLineNumber(), self._locator.getColumnNumber()
        self._logger.error(errorId, position, **kwargs)

    def _checkUnfinished(self):
        """
        Check for problems with unfinished translations.

        :return: True, if the translation is unfinished, False otherwise.
        """
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
        """ Check for problems with whitespaces. """
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
        """ Check for problems with newlines. """
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
        """ Check for problems with format specifiers. """
        for formatSpecifierRegex in self._formatSpecifierRegexes:
            sourceSpecifiers = set(formatSpecifierRegex.findall(self._source))
            translationSpecifiers = set(formatSpecifierRegex.findall(self._translation))
            for specifier in sourceSpecifiers ^ translationSpecifiers:
                if specifier in sourceSpecifiers:
                    self.error('format_specifier_missing',
                               self._translationPos, specifier=self._escapeText(specifier))
                else:
                    self.error('format_specifier_unbalanced', self._calculatePosition(
                        self._translation, self._translation.index(specifier),
                        self._translationPos), specifier=self._escapeText(specifier))

    def _checkPunctuation(self):
        """ Check for problems with punctuations. """
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
                    self._translation, len(self._translation) - 1, self._translationPos),
                             punctuation=self._escapeText(lastSourceChar),
                             actual=self._escapeText(lastTranslationChar))

    def _checkSpecialPatterns(self):
        """ Check for problems with special patterns. """
        for pattern in self._specialPatterns:
            for sourceMatch in pattern.finditer(self._source):
                text = sourceMatch.group('match')
                if text not in self._translation:
                    self.error('special_pattern_missing', self._translationPos,
                               pattern=self._escapeText(sourceMatch.group('match')))

    def _checkHtml(self):
        """ Check for problems with html in translations. """
        if '<html>' not in self._source or '</html>' not in self._source:
            return
        parser = HTMLComparer(self._logger, self._translationPos)
        parser.feed(self._source)
        parser.compare(self._translation)

    def _currentElement(self):
        return self._elementStack[-1]

    @staticmethod
    def _calculatePosition(string, index, filePos):
        """
        Calculate the position in the translation file given the start location
        of the xml element and an index into it's content.

        :param string: The content of the xml element.
        :param index: The index into that content.
        :param filePos: The start position of the content of the xml element.
        :return: The file position of the index in the xml element.
        """
        lineNo, column = filePos
        for line in string.split('\n'):
            length = len(line)
            if index <= length:
                return lineNo, column + index
            index -= length + 1
            lineNo += 1
            column = 0
        raise ValueError('Index is not in string')

    @staticmethod
    def _escapeText(text):
        """
        Escape the text so it can be printed without messing uo the error or warning format.
        :param text: THe text to escape.
        :return: THe escaped text.
        """
        return text.replace('\n', '\\n').replace('\t', '\\t')


def processFile(translationFile, formatSpecifierRegexes, specialPatterns, globalWarningFilter):
    """
    Process a translation file and generate warnings and errors.

    :param translationFile: The path to the translation file.
    :param formatSpecifierRegexes: A list of regexes for matching format specifiers.
    :param specialPatterns: A list of regexes for matching patterns
                            which must be equal in the source and translation.
    :param globalWarningFilter: A warning and error filter for the whole file.
    :return: The number of errors emitted.
    """
    print(f'Processing {translationFile}...')
    parser = make_parser()
    handler = TranslationHandler(
        translationFile, formatSpecifierRegexes, specialPatterns, globalWarningFilter)
    parser.setContentHandler(handler)
    try:
        parser.parse(translationFile)
    except SAXParseException as error:
        handler.error('xml_invalid', message=str(error))
    numErrors = handler.getNumErrors()
    print(f'Found {handler.getNumWarnings()} warning(s) and {numErrors} error(s)')
    return numErrors


def main(translationFiles, formatSpecifierRegexes, specialPatterns, warnings):
    """
    Lint all given translation files.

    :param translationFiles: A list of globs for translation files.
    :param formatSpecifierRegexes: A list of regexes for matching format specifiers.
    :param specialPatterns: A list of regexes for matching patterns
                            which must be equal in the source and translation.
    :param warnings: A warning and error filter for the whole file.
    :return: 1, if errors were found, 0 otherwise.
    """
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
    Parse the command line.

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
