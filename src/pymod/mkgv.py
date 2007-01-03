import generate

p = generate.FilteringParser(input='gv.defs',
                             prefix='gvmodule',
                             typeprefix='&')
p.startParsing()
