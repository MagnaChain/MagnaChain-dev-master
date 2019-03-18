#!/usr/bin/perl -w
use strict;
use autodie;
use File::Copy "cp";
use File::Temp qw(tempdir);
use Cwd;
use File::Path qw(make_path);
use File::Spec::Functions;

die "Permission denied!Need root.\n" unless `whoami` =~ /root/ig;
die "missing parameter\n" unless @ARGV >= 2;

my $projectRoot = $ARGV[0];
my $tmp = tempdir( CLEANUP => 1 );
my $appDir = catdir($tmp,"app");
my $mgc = catdir($projectRoot,'src/magnachaind');
my $cli = catdir($projectRoot,'src/magnachain-cli');
print "Get tmp folder:$tmp\n";
print `pwd`;
print "$mgc\n$cli";

die "magnachaind or magnachain-cli not exists\n" unless -e $mgc && -e $cli;

make_path($appDir);
cp($mgc,$appDir);
cp($cli,$appDir);

copyDep();

cp("Dockerfile",$tmp);
cp("docker-entrypoint.sh",$tmp);

if (@ARGV >= 2){
	my $dest = $ARGV[1];
	for (glob catfile($appDir,"*")){
		print "$_\n";
		cp($_,$dest);
	}
}

# buildAndPush();


sub copyDep {
	# body...
	my @soList;
	for (("ldd $mgc","ldd $cli")){
		print "do cmd:$_\n";
		my @cmdResult = `$_`;
		while (my $line = shift @cmdResult) {
	        chomp($line);
	        $line =~ s/\s+//ig;
	        print "found:$line\n";
	        my ($name,$path) = split '=>',$line;
	        if ($path && $path =~ /(.*)\(/ig){
	            push @soList,$1;    
	        }			    
		}    
	}
	for (@soList) {
	    # body...
    	cp($_,$appDir)||die "could not copy files :$!" ;
	}
}

sub buildAndPush {
	# body...
	my $tag = shift||'magnachain';
	my $ver = shift||'latest';
	my $build = "sudo docker build -t $tag .";
	my $push = "sudo docker tag $tag:$ver user/test:latest";

	chdir($tmp);
	!system $build or die "buildImage error:$!\n";
}

sub splitBin {
	# body...
	# 将magnachaind文件进行分割，避免github太大上传不了
	# #TODO
}














