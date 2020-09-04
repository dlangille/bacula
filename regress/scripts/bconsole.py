#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2020 Kern Sibbald

   The original author of Bacula is Kern Sibbald, with contributions
   from many others, a complete list can be found in the file AUTHORS.

   You may use this file and others of this release according to the
   license defined in the LICENSE file, which includes the Affero General
   Public License, v3.0 ("AGPLv3") and some additional permissions and
   terms pursuant to its AGPLv3 Section 7.

   This notice must be preserved when any source code is
   conveyed and/or propagated.

   Bacula(R) is a registered trademark of Kern Sibbald.
#
# author: alain@baculasystems.com

import os
import sys
import subprocess
import time
import re
import itertools
import codecs
import string
import threading

def grouper(iterable, n, fillvalue=None):
    args = [iter(iterable)] * n
    return itertools.izip_longest(*args, fillvalue=fillvalue)

class BConsoleError(RuntimeError):

    def __init__(self, exit_code, stdout, stderr):
        RuntimeError.__init__(self, 'Bconsole error')
        self.exit_code=exit_code
        self.stdout=stdout
        self.stderr=stderr

class BConsole():

    """
    
    Difference between linux and windows bconsole
    while running
    'gui on\n.client\n' 
    
    Windows bconsole return this
    -----------------------------------    
    Connecting to Director zbacula.asxnet.loc:9101
    1000 OK: zbacula.asxnet.loc-dir Version: 6.2.1 (18 January 2013)
    Enter a period to cancel a command.
    **zlucid-fd
    zozo
    zbacula.asxnet.loc-fd
    zwin2012-fd
    another-rescue-fd
    rescue-fd
    zwin2008r2-fd
    envy-fd
    zwin2003-fd
    *
    -----------------------------------    
    Linux bconsole return this
    -----------------------------------    
    Connecting to Director zbacula:9101
    1000 OK: zbacula.asxnet.loc-dir Version: 6.2.1 (18 January 2013)
    Enter a period to cancel a command.
    gui on
    .clients
    zlucid-fd
    zozo
    zbacula.asxnet.loc-fd
    zwin2012-fd
    another-rescue-fd
    rescue-fd
    zwin2008r2-fd
    envy-fd
    zwin2003-fd
    
    -----------------------------------
    
    Linux echo the command while windows display * for any '\n'  
    The * and the empty line is because of the '\n' at the end of the 'query'
        
    """
    
    def __init__(self, bin_path=None, conf_path=None, log=None, regress=None):
        # log is a callable, can be logging.debug
        if log in (None, False):
            log=lambda *args: None
        self.log=log

        if sys.platform.startswith('win'):
            bin_paths=[ r'C:\Program Files\Bacula\bconsole.exe', r'.\bconsole.exe' ]
            conf_paths=[ r'C:\Program Files\Bacula\bconsole.conf', r'.\bconsole.conf' ]
        else:
            if regress or \
               (os.getenv('BACULA_SOURCE', None) and os.getenv('BASEPORT', None) and \
               os.path.isfile('bin/bconsole') and os.path.isfile('bin/bconsole.conf')):
                # we are running a REGRESS test
                bin_paths=[ r'bin/bconsole', ]
                conf_paths=[ r'bin/bconsole.conf', ]
            else:
                bin_paths=[ '/usr/bin/bconsole', '/usr/local/bin/bconsole', '/opt/bacula/bin/bconsole', './bconsole' ]
                conf_paths=[ '/etc/bacula/bconsole.conf', '/opt/bacula/etc/bconsole.conf', './bconsole.conf' ]

        if not bin_path:
            for path in bin_paths:
                if os.path.isfile(path):
                    bin_path=path

        if not conf_path:
            for cp in conf_paths:
                if os.path.isfile(cp):
                    conf_path=cp
            
#        if bin_path==None:
#            raise RuntimeError, 'bconsole executable not found: %s' % (bin_path, )
        
#        if conf_path==None:
#            raise RuntimeError, 'bconsole configuration file not found: %s' % (conf_path, )
            
        self.bin_path=bin_path
        self.conf_path=conf_path
        self.bconsole_cmd=[ self.bin_path, '-n', '-c', self.conf_path]
        self.log('bconsole initialized: %r', self.bconsole_cmd)
        self.buf=''
        
    def rawrun0(self, cmds, nolog=False):
        if nolog:
            log=lambda *args: None
        else:
            log=self.log
            
        if not isinstance(cmds, (list, tuple)):
            cmds=[ cmds, ]

        command='\n'.join(cmds)
        
        log('running bconsole command: ', )
        for cmd in cmds:
            log('< %s', cmd)

        proc=subprocess.Popen(self.bconsole_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
        out, err=proc.communicate(input=codecs.encode(command))
        if err:
            for line in err.splitlines():
                log('ERR > %s', line)
        
        for line in out.splitlines():
            log('> %s', line)

        if proc.returncode!=0:
            log('bconsole exit_code=%d', proc.returncode)

        if proc.returncode!=0 or err:
            raise BConsoleError(proc.returncode, out, err)

        return out

    def ReadStd(self, fd, verbose, out):
        buffer=b''
        buf=os.read(fd, 4096)
        while buf:
            if verbose:
                os.write(sys.stdout.fileno(), buf)
            buffer+=buf
            buf=os.read(fd, 4096)
        out[0]=buffer

    def rawrun(self, cmds, verbose=False):
        if not isinstance(cmds, (list, tuple)):
            cmds=[ cmds, ]

        command='\n'.join(cmds)

        proc=subprocess.Popen(self.bconsole_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
        stdout=[ '' ]
        thread_out=threading.Thread(target=self.ReadStd, args=(proc.stdout.fileno(), verbose, stdout ))
        thread_out.start()
        stderr=[ '' ]
        thread_err=threading.Thread(target=self.ReadStd, args=(proc.stderr.fileno(), verbose, stderr))
        thread_err.start()
        proc.stdin.write(codecs.encode(command))
        proc.stdin.close()
        proc.wait()
        thread_out.join()
        thread_err.join()

        return proc.returncode, stdout[0], stderr[0]

    def simplerun(self, onecmd, nolog=False, input=None):
        """run a single line command and return filtered lines"""
        # "gui on" turn off the "You have messages" alerts 
        cmds=[ 'gui on', onecmd, '']
        if input:
            cmds.insert(2, input)
        returncode, out, err=self.rawrun(cmds, nolog)
        # the empty command at the end is important for linux to avoid a merge of
        # the echoed command and the answer 
        orig=out
        out=out.splitlines()
        # drop the 3 firsts lines [ 'Connection..', '1000 OK..', 'Enter a period'
        out=out[3:] 
        if sys.platform.startswith('win'):
            # drop the two ** at the beginning (one * per command)
            assert out[0][:2]=='**', "Expect ** in windows bconsole after the 3 'connection lines': %r" % (orig,)
            out[0]=out[0][2:]
            if out[-1]=='*':
                # drop the last line with only one *
                out=out[:-1]
            else:
                # then the * is at the end of the last line
                # this happen in SQL query
                out[-1]=out[-1][:-1]
        else:
            # drop the 2 echoed line [ 'gui on', onecmd ]
            out=out[2:]

        return out

    def get_greeting_banner(self, nolog=False):
        out=self.rawrun([ 'gui on', 'q', ''], nolog)
        line=out.splitlines()[1]
        # 1000 OK: zbacula.asxnet.loc-dir Version: 6.2.5 (20 May 2013).
        # 1000 OK: 1 zbacula.asxnet.loc-dir Version: 6.6.0 (04 Nov 2013).
        return line
  
    def list_clients(self, sort=False):
        """return the sorted (or not) list of clients"""
        lst=self.simplerun('.clients')
        if sort:
            lst.sort()
        return lst    
        
    
    def list_jobs_for_one_client(self, client_name):
        """return a list of (JobId, StartTime, Level, Name) both in ASCII"""
        sqlcmd="SELECT JobId, StartTime, Level, Job.Name " \
               "FROM Job JOIN Client USING (ClientId) " \
               "WHERE Client.Name = '%s' " \
               "AND Job.Type = 'B' AND Job.JobStatus IN ('T', 'W') " \
               "ORDER By JobTDate DESC" % (client_name,)
           
        out=self.simplerun('.sql query="%s;"' % (sqlcmd, ))
        catalog=out[0] # 'Using Catalog "MyCatalog"'
        if len(out)==1:
            return []
        result=out[1] # fields separated by TAB
        fields=result.split('\t')[:-1]
        
#        for jobid, date, start in grouper(fields, 3, None):
#            print jobid, time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(int(date))), start

        return list(grouper(fields, 4, None))

    def get_job_dependencies(self, jobid):
        return self.simplerun('.bvfs_get_jobids jobid='+jobid)[1]
    
    def bvfs_update(self, jobids):
        self.simplerun('.bvfs_update jobid='+jobids)
    
    def bvfs_lsdirs(self, jobids, path=None, pathid=None):
        cmd='.bvfs_lsdirs jobid='+jobids
        if path!=None:
            cmd+=' path='+path
        if pathid!=None:
            cmd+=' pathid='+pathid
        out=self.simplerun(cmd)
        dirs=[]
        for dir in out[1:]:
            items=dir.split('\t')
            if items[5] in ( '.', '..'):
                continue
            
            dirs.append((items[0], items[5]))
            
        return dirs

    def bvfs_lsfiles(self, jobids, path=None, pathid=None):
        cmd='.bvfs_lsfiles jobid='+jobids
        if path!=None:
            cmd+=' path='+path
        if pathid!=None:
            cmd+=' pathid='+pathid
        out=self.simplerun(cmd)
        files=[]
        for f in out[1:]:
            items=f.split('\t')
            files.append((items[2], items[5]))
            
        return files

        
    def bvfs_walk_path(self, jobids, path, pathid=None, caseinsensitive=False):
        # print 'bvfs_walk_path', path, pathid
        if not caseinsensitive:
            # if it is case sensitive then don't compare the upper() of the strings
            upper=lambda x:x
        else:
            upper=string.upper
        
        if pathid==None:
            dirs=self.bvfs_lsdirs(jobids, path='')
        else:
            dirs=self.bvfs_lsdirs(jobids, pathid=pathid)
            
        first=upper(path[0])
        if not first.endswith('/'):
            first+='/'
            
        for pathid, name in dirs:
            if first==upper(name):
                if len(path)==1:
                    return pathid
                else:
                    return self.bvfs_walk_path(jobids, path[1:], pathid=pathid, caseinsensitive=caseinsensitive)

        return None

    def restore(self, client, restoreclient, restorejob, **kwargs):
        """return the jobid (string) or None if job didn't start"""
        extra=' '.join([ k if v==None else '%s=%s' % (k, v) for k,v in kwargs.iteritems() ])
        cmd='restore client=%s restoreclient=%s restorejob=%s yes %s' % (client, restoreclient, restorejob, extra, )
        out=self.simplerun(cmd)
        try:
            jobid=re.search('Job queued. JobId=(\d+)', '\n'.join(out)).group(1)
        except AttributeError:
            jobid=None
        return jobid

    def restore_input(self, client, restoreclient, restorejob, input, **kwargs):
        """return the jobid (string) or None if job didn't start"""
        extra=' '.join([ k if v==None else '%s=%s' % (k, v) for k,v in kwargs.iteritems() ])
        cmd='restore client=%s restoreclient=%s restorejob=%s yes %s' % (client, restoreclient, restorejob, extra, )
        out=self.simplerun(cmd, input=input)
        try:
            jobid=re.search('Job queued. JobId=(\d+)', '\n'.join(out)).group(1)
        except AttributeError:
            jobid=None
        return jobid

    def search_simple_restore_jobs(self):
        """return a list of restore job without any runscript"""
        restore_jobs=self.simplerun('.jobs type=R')
        lst=[]
        for job in restore_jobs:
            description='\n'.join(self.simplerun('show job='+job))
            if not re.search('--> RunScript', description):
                lst.append(job)
        
        return lst
    
    def status_restore_job(self, cli_name, nolog=True):
        """
            Files=46,507 Bytes=12,875,215,455 AveBytes/sec=16,094,019 LastBytes/sec=16,094,019
            Bwlimit=0
            Files: Restored=0 Expected=66 Completed=0%

        """
        out=self.simplerun('status client=%s' % (cli_name, ), nolog)
        txt='\n'.join(out)
        if re.search('No Jobs running', txt):
            return None
        
        # I could have multiple jobs running on the same time
        # but this will never happend in winbmr
        try: 
            restored, expected, completed=re.search('Files: Restored=([.,0-9]+) Expected=([.,0-9]+) Completed=(\d+)', txt).groups()
        except AttributeError:
            restored, expected, completed=0, 0, 0
             
        return restored, expected, completed
        

#----------------------------------------------------------------------
if __name__ == "__main__":
    # try python bconsole.py .clients

    import logging
            
    log=logging.getLogger()
    log.setLevel(logging.INFO)
    console=logging.StreamHandler()
    log.addHandler(console)
    
    #bconsole=BConsole(bin_path=r'X:\Bacula\rescue\executables\bconsole.exe', conf_path=r'X:\Bacula\rescue\executables\bconsole.conf', log=log.info)
    # bconsole.run(sys.argv[1:], log.info)
    print(BConsole().list_clients())
    
    
    
