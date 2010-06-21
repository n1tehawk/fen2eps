# Python script for creating a Build version of fen2eps
# under Windows/Linux
#
# Author: Dirk Baechle
# 

import os, sys, shutil, glob

def ensureFilePathExists(fp):
    head, tail = os.path.split(fp)
    if not os.path.exists(head):
        plist = [head]
        while 1:
            head, tail = os.path.split(head)
            if head != '':
                plist.append(head)
            if tail == '':
                break
        plist.reverse()
        for p in plist:
            if not os.path.exists(p):
                os.mkdir(p)

def copyContents(srcdir, dstdir):
    """Copies the contents of the specified folder srcdir to 
    dstdir directory.
    If the dstdir does not exist, it is created first.
    """
    ensureFilePathExists(os.path.join(dstdir, 'dummy.txt'))

    for entry in os.listdir(srcdir):
        if entry == '.svn':
            continue
        
        epath = os.path.join(srcdir, entry)
        dpath = os.path.join(dstdir, entry)
        if os.path.isdir(epath):
            # Copy the subfolder
            shutil.copytree(epath, dpath)
        else:
            shutil.copy(epath, dpath)

def stripCommonPrefix(prefix, path):
    if path.startswith(prefix):
        return path[len(prefix):]
    return path

def addResourceFiles(dpath):
    # Files
    shutil.copy('../rsc/addons/fed/default.fed',dpath)
    # Directories
    shutil.copytree('../rsc/addons/examples', os.path.join(dpath,'examples'))
    shutil.copytree('../rsc/addons/fed/fed', os.path.join(dpath,'fed'))

# The FED keys we are interested in
fed_keys = ['FontName', 'FontVersion', 'FontDate', 'FontAuthor', 'Description']

def parseFEDFile(fpath):
    f = open(fpath,'r')
    
    info = {}
    PM_VOID = 0
    PM_INFO = 1
    PM_TAG = 2
    parse_mode = PM_VOID
    key = ''
    value = []
    for l in f.readlines():
        l = l.rstrip('\n')
        if parse_mode == PM_INFO:
            if l.startswith('%BEGIN'):
                parse_mode = PM_TAG
                key = ' '.join(l.split()[1:])
                value = []
            if l.startswith('%END FontInfo'):
                parse_mode = PM_VOID
        elif parse_mode == PM_TAG:
            if l.startswith('%END'):
                # Assign current key
                if key in fed_keys:
                    info[key] = '\n'.join(value)
                parse_mode = PM_INFO
            else:
                value.append(l)
        else:
            if l.startswith('%BEGIN FontInfo'):
                parse_mode = PM_INFO

    f.close()

    return info

default_ext = ['.fo','.xml','.xsl','.dblite','.wiki','.eps']

def stripBuildDir(dpath, rem_ext, r_files):
    svn_dirs = []
    for path, dirs, files in os.walk(dpath):
        for d in dirs:
            if d == '.svn':
                dp = os.path.join(path, d)
                if dp not in svn_dirs:
                    svn_dirs.append(dp)
    
    for s in svn_dirs:
        shutil.rmtree(s, ignore_errors=True)
    
    
    rem_files = []
    for path, dirs, files in os.walk(dpath):
        for f in files:
            root, ext = os.path.splitext(f)
            if (ext in rem_ext) or (f in r_files):
                fp = os.path.join(path, f)
                if fp not in rem_files:
                    rem_files.append(fp)

    for f in rem_files:
        os.remove(f)

def prepareBuildDir(dpath):
    # Copy sources
    copyContents('../src', os.path.join(dpath,'fen2eps'))
    # Add resource files
    addResourceFiles('%s/fen2eps' % dpath)

def cleanHomepage():
    shutil.rmtree('./homepage', ignore_errors=True)

def createHomepage():
    cleanHomepage()
    os.mkdir('./homepage')
    cwd = os.path.abspath(os.getcwd())
    os.chdir('homepage')
    os.system('forrest seed')
    os.chdir(cwd)
    # Compile docs
    copyContents('../www/xdocs','homepage/src/documentation/content/xdocs')
    copyContents('../doc/manual','homepage/src/documentation/content/xdocs/manual')
    copyContents('./docs/fontlist','homepage/src/documentation/content/xdocs/fontlist')
    copyContents('../www/ps','homepage/src/documentation/content/xdocs/ps')
    # Copy skin files
    shutil.copytree('../www/skins','homepage/src/documentation/skins')
    shutil.copy('../www/skinconf.xml','homepage/src/documentation')
    # Create homepage
    os.system('cd homepage/src/documentation/content/xdocs/; python ~/programming/python/xmlwiko/xmlwiko.py')
    os.chdir(cwd)
    os.chdir('homepage')
    os.system('forrest')
    os.chdir(cwd)

def cleanDocumentation():
    shutil.rmtree('./docs', ignore_errors=True)

def writeFontlistWiki():
    # detect fen2eps
    fen2eps = 'fen2eps'
    if os.path.exists('../../linux_exe/fen2eps/fen2eps'):
        fen2eps = '../../linux_exe/fen2eps/fen2eps'
    else:
        print "Warning: Using a (possibly) out-of-date version on fen2eps!"
        print "Advice: Call build.py without any arguments, for a full build."
        
    # create directory for board images
    os.mkdir('boards')    
        
    # Inside docs/fontlist folder
    f = open('fontlist.wiki','w')
    f.write("""@title: Fontlist for Fen2eps v1.1
@author: Dirk Baechle
@date: 2010-06-20

Abstract:
This text lists the various chess fonts, currently available
for \\\\Fen2eps\\\\. Please, note that the EPS files for the single
boards have been converted to the PNG format. This means that the
displayed images do not show the highest quality possible.
    
""")
    
    # Detect FED files
    feds = glob.glob('../../../rsc/addons/fed/fed/*.fed')
    
    fdict = {}
    for font in feds:
        info = parseFEDFile(font)
        fdict[info['FontName']] = [info, font]
        
    for k in sorted(fdict.keys()):
        info = fdict[k][0]
        font = fdict[k][1]
        # get file stem
        head, tail = os.path.split(font)
        stem, ext = os.path.splitext(tail)
        
        f.write("""== %s ==
$$%s.fed$$, by %s (%s)

Image: boards/%s.png||**docbook scale="45"** **forrest width="600" alt="%s.png"**

Raw:
 **docbook <?hard-pagebreak?>**

""" % (info['FontName'],stem,info['FontAuthor'],info['FontDate'],stem,stem))


        # create eps and png files
        os.system("%s -f %s < ../../../rsc/addons/examples/test.fen > boards/%s.eps" % (fen2eps, font, stem))
        os.system("gs -sDEVICE=ppm -sPAPERSIZE=a6 -q -dNOPAUSE -sOutputFile=boards/%s.ppm -dBATCH -r300 boards/%s.eps" % (stem, stem))
        os.system("cat boards/%s.ppm | pnmcrop -white | pnmtopng > boards/%s.png" % (stem, stem))
        os.remove('boards/%s.ppm' % stem)
        
    f.close()
    
def createDocumentation():
    cleanDocumentation()
    os.mkdir('./docs')
    cwd = os.path.abspath(os.getcwd())
    os.chdir('docs')
    # Compile docs
    copyContents('../../doc/manual','manual')
    os.mkdir('fontlist')
    os.chdir('fontlist')
    writeFontlistWiki()
    os.chdir('..')
    os.system('python ~/programming/python/xmlwiko/xmlwiko.py db')
    
    copyContents('../db_scons','manual')
    copyContents('../db_scons','fontlist')

    os.chdir('manual')
    os.system('scons manual')
    os.chdir('../fontlist')
    os.system('scons fontlist')
    os.chdir(cwd)

def cleanLinux():
    shutil.rmtree('./linux', ignore_errors=True)
    shutil.rmtree('./linux_exe', ignore_errors=True)

def createLinux():
    cleanLinux()
    prepareBuildDir('linux')
    prepareBuildDir('linux_exe')

def cleanWindows():
    shutil.rmtree('./windows_exe', ignore_errors=True)

def createWindows():
    cleanWindows()
    prepareBuildDir('windows_exe')

def finishLinux():
    # Copy manual and fontlist
    copyContents('docs/manual','linux/fen2eps/manual')
    copyContents('docs/fontlist','linux/fen2eps/fontlist')
    stripBuildDir('linux/fen2eps', default_ext, [])
    stripBuildDir('linux/fen2eps/manual', default_ext, ['SConstruct'])
    stripBuildDir('linux/fen2eps/fontlist', default_ext, ['SConstruct'])

def finishWindows():
    # Copy manual and fontlist
    copyContents('docs/manual','windows_exe/fen2eps/manual')
    copyContents('docs/fontlist','windows_exe/fen2eps/fontlist')
    stripBuildDir('windows_exe/fen2eps', default_ext, [])
    stripBuildDir('windows_exe/fen2eps/manual', default_ext, ['SConstruct'])
    stripBuildDir('windows_exe/fen2eps/fontlist', default_ext, ['SConstruct'])

def main():
    if os.sys.platform.find('linux') >= 0:
        if ((len(sys.argv) > 1) and (sys.argv[1] == 'clean')):
            cleanLinux()
            cleanWindows()
            cleanHomepage()
            cleanDocumentation()
        else:
            createLinux()
            createWindows()
            os.system("make -C linux_exe/fen2eps")
            createDocumentation()
            createHomepage()
            finishLinux()
            finishWindows()
    else:
        createWindows()

if __name__ == '__main__':
    main()

