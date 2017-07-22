##############################################
# $Id: $
package main;

use strict;
use warnings;
use SetExtensions;

my %sets = (
  "clear:noArg"         => "",
  "time:noArg"          => "",
  "raw"                 => "",
);

sub
Betty_Initialize($)
{
  my ($hash) = @_;


  $hash->{Match}     = "^y.*";
  $hash->{SetFn}     = "Betty_Set";
  $hash->{DefFn}     = "Betty_Define";
  $hash->{ParseFn}   = "Betty_Parse";
  $hash->{AttrList}  = "IODev ". $readingFnAttributes;
}



###################################
sub
Betty_Set($@) {
  my ($hash, @a) = @_;

  return "set $hash->{NAME} needs at least one parameter" if(@a < 2);

  my $me   = shift @a;
  my $cmd  = shift @a;
  my $arg  = shift @a;
  my $arg2 = shift @a;
  
  return join(" ", sort keys %sets) if ($cmd eq "?");

  if ($cmd eq "clear") { 
    my @cH = ($hash);
    delete $_->{READINGS} foreach (@cH);
    return undef;

  } elsif ($cmd eq "raw") { 
    my $msg = $arg;
    #IOWrite( $hash, $msg );
    IOWrite( $hash, "y", $msg );
    return undef;

  } elsif ($cmd eq "time") {
    my $address = $hash->{ADDRESS};
    
    my ($sec,$min,$hour,$mday,$month,$year,$wday,$yday,$isdst) = localtime;
    
    my $m = sprintf("%02d%02d%02d%02d",$mday,$wday,$month+1, $year-100);
    my $n = sprintf("%02d%02d%02d",$sec, $min, $hour );
     
    my $msg = "w".$address;
    IOWrite( $hash, "y", $msg );
    $msg = "s0a".$address."0003".$m.$n;
    IOWrite( $hash, "y", $msg );
    
    return undef;

  }

  return "Unknown argument $cmd, choose one of ". join(" ", sort keys %sets); 
}


#############################
sub
Betty_Define($$)
{
  my ($hash, $def) = @_;
  my @a = split("[ \t][ \t]*", $def);

  return "wrong syntax: define <name> Betty <address>"
            if(int(@a) < 2 || int(@a) > 4);
  
  my $address = uc($a[2]);
  $hash->{ADDRESS} = $address;
  $modules{Betty}{defptr}{$address} = $hash;
  AssignIoPort($hash);
  
  readingsSingleUpdate($hash, "state", "Initialized", 1);
  
  return undef;
}


sub
Betty_Parse($$)
{
  my ($hash,$msg) = @_;

  my ($len,$dest,$src,$service,$data) = unpack 'x1A2A2A2A2A*',$msg;

  my $def = $modules{Betty}{defptr}{$src};
  
  if(!$def) {
    DoTrigger("global","UNDEFINED Betty_$src Betty $src");
    $def = $modules{Betty}{defptr}{$src};
    if(!$def) {
      Log3 $hash, 1, "Betty UNDEFINED, address $src";
      return "UNDEFINED Betty_$src Betty $src";
    }
  }

  $hash = $def;
  my $name = $hash->{NAME};  
  
  # packet_RFenc
  if ($service eq '04') {       
    my ($addr,$key) = unpack 'A2A2',$data;
    
    if($hash->{helper}{lastkey } ne $key) {
      $hash->{helper}{lastkey } = $key;
      $key = sprintf "%02x", hex($key) & 0x7F;   
      readingsSingleUpdate($hash, "key", $addr."_".$key  , 1);
    }
    
  
  # packet_test
  } elsif ($service eq '01') {
    
    $data = latin1ToUtf8(pack("H*",$data));     
    readingsSingleUpdate($hash, "test", $data , 1);
    
  # packet_time
  } elsif ($service eq '03') {
    
    my ($request) = unpack 'A2',$data;
    
    if($request eq "FF") {
      my ($sec,$min,$hour,$mday,$month,$year,$wday,$yday,$isdst) = localtime;
      
      my $m = sprintf("%02d%02d%02d%02d",$mday,$wday,$month+1, $year-100);
      my $n = sprintf("%02d%02d%02d",$sec, $min, $hour );
       
      $msg = "s0a".$src."0003".$m.$n;
      IOWrite( $hash, "y", $msg );
    }
    
  } else {
    Log3 $hash, 4, "Betty UNKNOWN MESSAGE $service: $data";
  }
  
  return $name;
}




1;

=pod
=item summary    devices communicating with the Betty remote control
=item summary_DE Anbindung der Betty Fernbedienung
=begin html

<a name="Betty"></a>
<h3>Betty</h3>
<ul>
  Todo
</ul>

=end html


=cut
