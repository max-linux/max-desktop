#-*- Mode: perl; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-

# Hosts Configuration handling
#
# Copyright (C) 2000-2001 Ximian, Inc.
#
# Authors: Hans Petter Jansson <hpj@ximian.com>
#          Carlos Garnacho     <carlosg@gnome.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

package Network::Hosts;

sub run_hostname
{
  my ($hostname) = @_;

  &Utils::Report::enter ();
  &Utils::Report::do_report ("network_hostname_set", $hostname);
  &Utils::File::run ("hostname", $hostname);
  &Utils::Report::leave ();
}

sub get_fqdn_dist
{
  my %dist_map =
	 (
    "debian"          => "debian",
    "redhat-6.2"      => "redhat-6.2",
    "redhat-7.0"      => "redhat-6.2",
    "redhat-7.1"      => "redhat-6.2",
    "redhat-7.2"      => "redhat-7.2",
    "redhat-8.0"      => "redhat-7.2",
    "mandrake-9.0"    => "redhat-6.2",
    "yoper-2.2"       => "redhat-6.2",
    "conectiva-9"     => "redhat-6.2", 
    "suse-9.0"        => "suse-9.0",
    "pld-1.0"         => "redhat-6.2",
    "vine-3.0"        => "redhat-6.2",
    "ark"             => "redhat-6.2",
    "slackware-9.1.0" => "suse-9.0",
    "gentoo"          => "gentoo",
    "freebsd-5"       => "freebsd-5",
    "solaris-2.11"    => "solaris-2.11",
	  );

  return $dist_map{$Utils::Backend::tool{"platform"}};
}

sub get_fqdn_parse_table
{
  my %dist_tables =
    (
     "redhat-6.2" =>
     {
       fn =>
       {
         SYSCONFIG_NW => "/etc/sysconfig/network",
         RESOLV_CONF  => "/etc/resolv.conf"
       },
       table =>
       [
        [ "hostname", \&Utils::Parse::get_sh, SYSCONFIG_NW, HOSTNAME ],
        [ "domain",   \&Utils::Parse::split_first_str, RESOLV_CONF, "domain", "[ \t]+" ]
       ]
     },

     "redhat-7.2" =>
     {
       fn =>
       {
         SYSCONFIG_NW => ["/etc/sysconfig/networking/profiles/default/network",
                          "/etc/sysconfig/networking/network",
                          "/etc/sysconfig/network"],
         RESOLV_CONF  => ["/etc/sysconfig/networking/profiles/default/resolv.conf",
                          "/etc/resolv.conf"],
       },
       table =>
       [
		    [ "hostname", \&Utils::Parse::get_sh, SYSCONFIG_NW, HOSTNAME ],
		    [ "domain",   \&Utils::Parse::split_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
		   ]
     },

     "debian" =>
     {
       fn =>
       {
         RESOLV_CONF => "/etc/resolv.conf",
         HOSTNAME    => "/etc/hostname",
       },
       table =>
       [
        [ "hostname", \&Utils::Parse::get_first_line, HOSTNAME ],
        [ "domain",	\&Utils::Parse::split_first_str, RESOLV_CONF, "domain", "[ \t]+" ]
       ]
     },

     "suse-9.0" =>
     {
       fn =>
       {
         RESOLV_CONF  => "/etc/resolv.conf",
         HOSTNAME     => "/etc/HOSTNAME",
       },
       table =>
       [
        [ "hostname", \&Utils::Parse::get_fq_hostname, HOSTNAME ],
        [ "domain", \&Utils::Parse::get_fq_domain, HOSTNAME ],
        [ "domain", \&Utils::Parse::split_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },
    
     "gentoo" =>
     {
       fn =>
       {
         HOSTNAME    => "/etc/conf.d/hostname",
         DOMAINNAME  => "/etc/conf.d/domainname",
         RESOLV_CONF => "/etc/resolv.conf",
       },
       table =>
       [
        [ "hostname", \&Utils::Parse::get_sh, HOSTNAME, HOSTNAME ],
        [ "domain", \&Utils::Parse::get_sh, DOMAINNAME, DNSDOMAIN ],
        [ "domain", \&Utils::Parse::split_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },

     "freebsd-5" =>
     {
       fn =>
       {
         RC_CONF     => "/etc/rc.conf",
         RESOLV_CONF => "/etc/resolv.conf",
       },
       table =>
       [
        [ "hostname", \&Utils::Parse::get_sh_re, RC_CONF, hostname, "^([^\.]*)\." ],
        [ "domain", \&Utils::Parse::split_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },

     "solaris-2.11" =>
     {
       fn =>
       {
         NODENAME    => "/etc/nodename",
         RESOLV_CONF => "/etc/resolv.conf",
       },
       table =>
       [
        [ "hostname", \&Utils::Parse::get_first_line, NODENAME ],
        [ "domain", \&Utils::Parse::split_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },
   );

  my $dist = &get_fqdn_dist ();
  return %{$dist_tables{$dist}} if $dist;

  &Utils::Report::do_report ("platform_no_table", $Utils::Backend::tool{"platform"});
  return undef;
}

sub get_fqdn_replace_table
{
  my %dist_tables =
    (
     "redhat-6.2" =>
     {
       fn =>
       {
         SYSCONFIG_NW => "/etc/sysconfig/network",
         RESOLV_CONF  => "/etc/resolv.conf"
       },
       table =>
       [
        [ "hostname", \&Utils::Replace::set_sh, SYSCONFIG_NW, HOSTNAME ],
        [ "hostname", \&run_hostname ],
        [ "domain", \&Utils::Replace::set_sh, SYSCONFIG_NW, DOMAIN ],
        [ "domain", \&Utils::Replace::join_first_str, RESOLV_CONF, "domain", "[ \t]+" ]
       ]
     },

     "redhat-7.2" =>
     {
       fn =>
       {
         SYSCONFIG_NW => ["/etc/sysconfig/networking/profiles/default/network",
                          "/etc/sysconfig/networking/network",
                          "/etc/sysconfig/network"],
         RESOLV_CONF  => ["/etc/sysconfig/networking/profiles/default/resolv.conf",
                          "/etc/resolv.conf"],
       },
       table =>
       [
		    [ "hostname", \&Utils::Replace::set_sh, SYSCONFIG_NW, HOSTNAME ],
        [ "hostname", \&run_hostname ],
        [ "domain", \&Utils::Replace::set_sh, SYSCONFIG_NW, DOMAIN ],
		    [ "domain", \&Utils::Replace::join_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
		   ]
     },

     "debian" =>
     {
       fn =>
       {
         RESOLV_CONF => "/etc/resolv.conf",
         HOSTNAME    => "/etc/hostname",
       },
       table =>
       [
        [ "hostname", \&Utils::Replace::set_first_line, HOSTNAME ],
        [ "hostname", \&run_hostname ],
        [ "domain",	\&Utils::Replace::join_first_str, RESOLV_CONF, "domain", "[ \t]+" ]
       ]
     },

     "suse-9.0" =>
     {
       fn =>
       {
         RESOLV_CONF  => "/etc/resolv.conf",
         HOSTNAME     => "/etc/HOSTNAME",
       },
       table =>
       [
        [ "hostname", \&Utils::Replace::set_fq_hostname, HOSTNAME, "%hostname%", "%domain%" ],
        [ "hostname", \&run_hostname ],
        [ "domain", \&Utils::Replace::join_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },
    
     "gentoo" =>
     {
       fn =>
       {
         HOSTNAME    => "/etc/conf.d/hostname",
         DOMAINNAME  => "/etc/conf.d/domainname",
         RESOLV_CONF => "/etc/resolv.conf",
       },
       table =>
       [
        [ "hostname", \&Utils::Replace::set_sh, HOSTNAME, HOSTNAME ],
        [ "hostname", \&run_hostname ],
        [ "domain", \&Utils::Replace::set_sh, DOMAINNAME, DNSDOMAIN ],
        [ "domain", \&Utils::Replace::join_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },

     "freebsd-5" =>
     {
       fn =>
       {
         RC_CONF     => "/etc/rc.conf",
         RESOLV_CONF => "/etc/resolv.conf",
       },
       table =>
       [
        [ "hostname", \&Utils::Replace::set_sh, RC_CONF, hostname, "%hostname%.%domain%" ],
        [ "hostname", \&run_hostname, "%hostname%.%domain%" ],
        [ "domain", \&Utils::Replace::join_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },

     "solaris-2.11" =>
     {
       fn =>
       {
         NODENAME    => "/etc/nodename",
         RESOLV_CONF => "/etc/resolv.conf",
       },
       table =>
       [
        [ "hostname", \&Utils::Replace::set_first_line, NODENAME ],
        [ "domain", \&Utils::Replace::join_first_str, RESOLV_CONF, "domain", "[ \t]+" ],
       ]
     },
   );

  my $dist = &get_fqdn_dist ();
  return %{$dist_tables{$dist}} if $dist;

  &Utils::Report::do_report ("platform_no_table", $Utils::Backend::tool{"platform"});
  return undef;
}

sub add_statichost_alias
{
  my ($localhost, $alias) = @_;
  my $i;

  foreach $i (@$localhost)
  {
    return if ($i eq $alias);
  }
  
  push @$localhost, $alias;
}

sub remove_statichost_alias
{
  my ($localhost, $alias) = @_;
  my $i;

  for ($i = 0; $i < @$localhost; $i++) {
    if ($$localhost[$i] eq $alias)
    {
      delete $$localhost[$i];
      return;
    }
  }
}
  
sub ensure_loopback_statichost
{
  my ($statichost, $hostname, $old_hostname, $lo_ip) = @_;
  my $i;

  if (exists $$statichost{$lo_ip})
  {
    my $localhost = $$statichost{$lo_ip};
    &remove_statichost_alias ($localhost, $old_hostname) if ($old_hostname);
    &add_statichost_alias ($localhost, $hostname);
  }
  else
  {
    $$statichost{$lo_ip} = [ ("localhost", "localhost.localdomain", $hostname) ];
  }
}

sub get_fqdn
{
  my %dist_attrib;
  my $hash;

  %dist_attrib = &get_fqdn_parse_table ();

  $hash = &Utils::Parse::get_from_table ($dist_attrib{"fn"},
                                         $dist_attrib{"table"});

  return ($$hash {"hostname"}, $$hash{"domain"});
}

sub parse_hosts_files
{
  my ($file) = @_;
  my (@arr, %hash, $statichosts, $i);

  while (@_)
  {
    $statichosts = &Utils::Parse::split_hash (@_[0], "[ \t]+", "[ \t]+");
    shift @_;

    foreach $i (keys %$statichosts)
    {
      $hash{$i} = $$statichosts{$i};
    }
  }

  foreach $i (sort keys %hash)
  {
    push @arr, [$i, $hash{$i}];
  }

  return \@arr;
}

sub get_hosts
{
  return &parse_hosts_files ("/etc/hosts", "/etc/inet/ipnodes") if ($Utils::Backend::tool{"system"} eq "SunOS");
  return &parse_hosts_files ("/etc/hosts");
}

sub get_dns
{
  my (@dns);

  @dns = &Utils::Parse::split_all_unique_hash_comment ("/etc/resolv.conf", "nameserver", "[ \t]+");

  return @dns;
}

sub get_search_domains
{
  my (@search_domains);

  @search_domains = &Utils::Parse::split_first_array_unique ("/etc/resolv.conf", "search", "[ \t]+", "[ \t]+");

  return @search_domains;
}

sub set_fqdn
{
  my ($hostname, $domain) = @_;
  my (%dist_attrib, %hash, %old_hash);

  $hash{"hostname"} = $hostname;
  $hash{"domain"} = $domain;

  ($old_hash{"hostname"}, $old_hash{"domain"}) = &get_fqdn ();

  %dist_attrib = &get_fqdn_replace_table ();
  &Utils::Replace::set_from_table ($dist_attrib{"fn"}, $dist_attrib{"table"},
                                   \%hash, \%old_hash);
}

sub ensure_hostname
{
  my ($hosts, $hostname, $domain, $old_hostname, $olddomain) = @_;
  my ($fqdn, $old_fqdn, $i);

  $fqdn  = $hostname;
  $fqdn .= ".$domain" if ($domain);

  $old_fqdn  = $old_hostname;
  $old_fqdn .= ".$old_domain" if ($old_domain);

  foreach $i (@$hosts)
  {
    if ($i eq $old_fqdn)
    {
      $i = $fqdn;
    }
  }
}

sub set_hosts
{
  my ($config, $hostname, $domain) = @_;
  my ($old_hostname, $old_domain) = &get_fqdn ();
  my ($i, %hash);

  foreach $i (@$config)
  {
    &ensure_hostname ($$i[1], $hostname, $domain, $old_hostname, $old_domain);
    $hash{$$i[0]} = $$i[1];
  }

  if ($Utils::Backend::tool {"system"} eq "SunOS")
  {
    &Utils::Replace::join_hash ("/etc/inet/ipnodes", "[ \t]+", "[ \t]+", \%hash);

    # save only IPv4 entries in /etc/hosts
    foreach $i (keys %hash)
    {
      delete $hash{$i} if ($i =~ /[a-fA-F:]/);
    }

    &Utils::Replace::join_hash ("/etc/hosts", "[ \t]+", "[ \t]+", \%hash);
  }
  else
  {
    &Utils::Replace::join_hash ("/etc/hosts", "[ \t]+", "[ \t]+", \%hash);
  }
}

sub set_dns
{
  my ($dns) = @_;

  &Utils::Replace::join_all ("/etc/resolv.conf", "nameserver", "[ \t]+", $dns);
}

sub set_search_domains
{
  my ($search_domains) = @_;

  &Utils::Replace::join_first_array ("/etc/resolv.conf", "search",
                                     "[ \t]+", "[ \t]+", $search_domains);
}

sub get_files
{
  my (%dist_attrib, $fn, $f, $files);

  %dist_attrib = &get_fqdn_parse_table ();
  $fn = $dist_attrib {"fn"};

  foreach $f (values %$fn)
  {
    push @$files, $f;
  }

  return $files;
}

1;
