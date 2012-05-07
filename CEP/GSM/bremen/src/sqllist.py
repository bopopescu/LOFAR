#!/usr/bin/python
"""
A set of tools to get queries from sql-files.
SQL-file has to contain separators of the form:
    --#query_name
Then query_name can be passed to get_sql as the parameter.
%s or {..} syntax is supported for substitutions.
Parameters for substitution are passed to get_sql.
"""
from os import path
import re
from src.queries import _get_assoc_r, _get_distance, _get_column_update

SQL_LIST = {}


def _expand_value(value):
    """
    Replace all occurences of $$function()$$ by result of the function call.
    """
    def _expand_formula(matchvalues):
        return str(eval(matchvalues.group(0)[2:-2]))
    return re.sub(r'\$\$(.*?)\$\$', _expand_formula, value, count=0)


def _load_from_sql_list(filename):
    """
    Load sql-commands from file.
    Command name is prefixed by --#
    """
    sqlfile = open(filename, 'r')
    hashkey = None
    hashvalue = ''
    for line in iter(sqlfile.readline, ''):
        if (line.startswith('--#')):
            if hashkey:
                SQL_LIST[hashkey] = _expand_value(hashvalue)
                hashvalue = ''
            hashkey = line[3:].strip()
        elif (not line.startswith('--')) and line:
            hashvalue = '%s %s' % (hashvalue, line.strip())
    if hashkey:
        SQL_LIST[hashkey] = _expand_value(hashvalue)
    print 'Loaded SQL list'
    sqlfile.close()


def get_sql(name, *params):
    """
    Returns an sql from the list by it's name with parameter substitution.
    """
    if not name in SQL_LIST:
        raise IndexError('Name %s not in sqllist.sql' % name)
    if (SQL_LIST[name].find('%') >= 0):
        return SQL_LIST[name] % (params)
    else:
        return SQL_LIST[name].format(*params)


_load_from_sql_list(path.dirname(path.abspath(__file__)) + '/sqllist.sql')
_load_from_sql_list(path.dirname(path.abspath(__file__)) + '/sqllist_api.sql')