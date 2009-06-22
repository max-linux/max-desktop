# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Utility code for easy input and output of XML.

The current implementation uses `xml.dom.minidom` under the hood for parsing.
"""

import os
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO
from UserDict import DictMixin

import cgi
import string

__trans = string.maketrans ("", "")
__todel = ""
for c in range (0, 256):
    c1 = chr (c)
    if not c1 in string.printable:
        __todel += c1
del c, c1

__all__ = ['Fragment', 'Element', 'ParsedElement', 'parse']
__docformat__ = 'restructuredtext en'

def _escape_text(text):
    """Escape special characters in the provided text so that it can be safely
    included in XML text nodes.
    """
    return cgi.escape (str(text)).translate (__trans, __todel)

def _escape_attr(attr):
    """Escape special characters in the provided text so that it can be safely
    included in XML attribute values.
    """
    return _escape_text(attr).replace('"', '&#34;')


class Fragment(object):
    """A collection of XML elements."""
    __slots__ = ['children']

    def __init__(self):
        """Create an XML fragment."""
        self.children = []

    def __getitem__(self, nodes):
        """Add nodes to the fragment."""
        if not isinstance(nodes, (list, tuple)):
            nodes = [nodes]
        for node in nodes:
            self.append(node)
        return self

    def __str__(self):
        """Return a string representation of the XML fragment."""
        buf = StringIO()
        self.write(buf)
        return buf.getvalue()

    def append(self, node):
        """Append an element or fragment as child."""
        if isinstance(node, Element):
            self.children.append(node)
        elif isinstance(node, Fragment):
            self.children += node.children
        elif node is not None and node != '':
            self.children.append(str(node))

    def write(self, out, newlines=False):
        """Serializes the element and writes the XML to the given output
        stream.
        """
        for child in self.children:
            if isinstance(child, (Element, ParsedElement)):
                child.write(out, newlines=newlines)
            else:
                if child.startswith('<'):
                    out.write('<![CDATA[' + child + ']]>')
                else:
                    out.write(_escape_text(child))


class Element(Fragment):
    """Simple XML output generator based on the builder pattern.

    Construct XML elements by passing the tag name to the constructor:

    >>> print Element('foo')
    <foo/>

    Attributes can be specified using keyword arguments. The values of the
    arguments will be converted to strings and any special XML characters
    escaped:

    >>> print Element('foo', bar=42)
    <foo bar="42"/>
    >>> print Element('foo', bar='1 < 2')
    <foo bar="1 &lt; 2"/>
    >>> print Element('foo', bar='"baz"')
    <foo bar="&#34;baz&#34;"/>

    The order in which attributes are rendered is undefined.

    Elements can be using item access notation:

    >>> print Element('foo')[Element('bar'), Element('baz')]
    <foo><bar/><baz/></foo>

    Text nodes can be nested in an element by using strings instead of elements
    in item access. Any special characters in the strings are escaped
    automatically:

    >>> print Element('foo')['Hello world']
    <foo>Hello world</foo>
    >>> print Element('foo')[42]
    <foo>42</foo>
    >>> print Element('foo')['1 < 2']
    <foo>1 &lt; 2</foo>

    This technique also allows mixed content:

    >>> print Element('foo')['Hello ', Element('b')['world']]
    <foo>Hello <b>world</b></foo>

    Finally, text starting with an opening angle bracket is treated specially:
    under the assumption that the text actually contains XML itself, the whole
    thing is wrapped in a CDATA block instead of escaping all special characters
    individually:

    >>> print Element('foo')['<bar a="3" b="4"><baz/></bar>']
    <foo><![CDATA[<bar a="3" b="4"><baz/></bar>]]></foo>
    """
    __slots__ = ['name', 'attr']

    def __init__(self, name_, **attr):
        """Create an XML element using the specified tag name.
        
        The tag name must be supplied as the first positional argument. All
        keyword arguments following it are handled as attributes of the element.
        """
        Fragment.__init__(self)
        self.name = name_
        self.attr = dict([(name, value) for name, value in attr.items()
                          if value is not None])

    def write(self, out, newlines=False):
        """Serializes the element and writes the XML to the given output
        stream.
        """
        out.write('<')
        out.write(self.name)
        for name, value in self.attr.items():
            out.write(' %s="%s"' % (name, _escape_attr(value)))
        if self.children:
            out.write('>')
            Fragment.write(self, out, newlines)
            out.write('</' + self.name + '>')
        else:
            out.write('/>')
        if newlines:
            out.write(os.linesep)


class ParseError(Exception):
    """Exception thrown when there's an error parsing an XML document."""


def parse(text_or_file):
    """Parse an XML document provided as string or file-like object.
    
    Returns an instance of `ParsedElement` that can be used to traverse the
    parsed document.
    """
    from xml.dom import minidom
    from xml.parsers import expat
    try:
        if isinstance(text_or_file, (str, unicode)):
            dom = minidom.parseString(text_or_file)
        else:
            dom = minidom.parse(text_or_file)
        return ParsedElement(dom.documentElement)
    except expat.error, e:
        raise ParseError(e)


class ParsedElement(object):
    """Representation of an XML element that was parsed from a string or
    file.
    
    This class should not be used directly. Rather, XML text parsed using
    `xmlio.parse()` will return an instance of this class.
    
    >>> xml = parse('<root/>')
    >>> print xml.name
    root
    
    Parsed elements can be serialized to a string using the `write()` method:
    
    >>> import sys
    >>> parse('<root></root>').write(sys.stdout)
    <root/>
    
    For convenience, this is also done when coercing the object to a string
    using the builtin ``str()`` function, which is used when printing an
    object:
    
    >>> print parse('<root></root>')
    <root/>
    
    (Note that serializing the element will produce a normalized representation
    that may not excatly match the input string.)
    
    Attributes are accessed via the `attr` member:
    
    >>> print parse('<root foo="bar"/>').attr['foo']
    bar
    
    Attributes can also be updated, added or removed:
    
    >>> xml = parse('<root foo="bar"/>')
    >>> xml.attr['foo'] = 'baz'
    >>> print xml
    <root foo="baz"/>

    >>> del xml.attr['foo']
    >>> print xml
    <root/>

    >>> xml.attr['foo'] = 'bar'
    >>> print xml
    <root foo="bar"/>

    CDATA sections are included in the text content of the element returned by
    `gettext()`:
    
    >>> xml = parse('<root>foo<![CDATA[ <bar> ]]>baz</root>')
    >>> xml.gettext()
    'foo <bar> baz'
    """
    __slots__ = ['_node', 'attr']

    class _Attrs(DictMixin):
        """Simple wrapper around the element attributes to provide a dictionary
        interface."""
        def __init__(self, node):
            self._node = node
        def __getitem__(self, name):
            attr = self._node.getAttributeNode(name)
            if not attr:
                raise KeyError(name)
            return attr.value.encode('utf-8')
        def __setitem__(self, name, value):
            self._node.setAttribute(name, value)
        def __delitem__(self, name):
            self._node.removeAttribute(name)
        def keys(self):
            return [key.encode('utf-8') for key in self._node.attributes.keys()]

    def __init__(self, node):
        self._node = node
        self.attr = ParsedElement._Attrs(node)

    name = property(fget=lambda self: self._node.localName,
                    doc='Local name of the element')
    namespace = property(fget=lambda self: self._node.namespaceURI,
                         doc='Namespace URI of the element')

    def children(self, name=None):
        """Iterate over the child elements of this element.

        If the parameter `name` is provided, only include elements with a
        matching local name. Otherwise, include all elements.
        """
        for child in [c for c in self._node.childNodes if c.nodeType == 1]:
            if name in (None, child.tagName):
                yield ParsedElement(child)

    def __iter__(self):
        return self.children()

    def gettext(self):
        """Return the text content of this element.
        
        This concatenates the values of all text and CDATA nodes that are
        immediate children of this element.
        """
        return ''.join([c.nodeValue.encode('utf-8')
                        for c in self._node.childNodes
                        if c.nodeType in (3, 4)])

    def write(self, out, newlines=False):
        """Serializes the element and writes the XML to the given output
        stream.
        """
        self._node.writexml(out, newl=newlines and '\n' or '')

    def __str__(self):
        """Return a string representation of the XML element."""
        buf = StringIO()
        self.write(buf)
        return buf.getvalue()


if __name__ == '__main__':
    import doctest
    doctest.testmod()
