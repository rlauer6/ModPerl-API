#!/usr/bin/env perl

use strict;
use warnings;

use Carp;
use DBI;
use Data::Dumper;
use English qw(no_match_vars);
use Getopt::Long qw(:config no_ignore_case);
use Pod::Usage;
use Text::ASCIITable::EasyTable;

use Readonly;

Readonly::Scalar our $TRUE  => 1;
Readonly::Scalar our $FALSE => 0;

Readonly::Scalar our $DEFAULT_DB_HOST => '127.0.0.1';
Readonly::Scalar our $DEFAULT_DB_USER => 'root';

Readonly::Scalar our $DEFAULT_ADMIN_USER  => 'blog-admin';
Readonly::Scalar our $DEFAULT_ADMIN_EMAIL => 'rclauer@gmail.com';

########################################################################
sub usage {
########################################################################
  return pod2usage( { -exitval => 1, -verbose => 1 } );
}

########################################################################
sub connect_db {
########################################################################
  my ($options) = @_;

  my $user = $options->{'db-user'} // $ENV{DBI_USER};
  my $pass = $options->{'db-pass'} // $ENV{DBI_PASS};
  my $host = $options->{'db-host'} // $ENV{DBI_HOST};

  my $dsn = sprintf 'dbi:mysql::%s;mysql_ssl=1', $host;

  return DBI->connect( $dsn, $user, $pass, { RaiseError => 1 } );
}

########################################################################
sub create_database {
########################################################################
  my ( $dbi, $options ) = @_;

  return
    if !$options->{create};

  print {*STDERR} "creating new database\n";

  $dbi->do('DROP DATABASE IF EXISTS blog_comments');

  $dbi->do('CREATE DATABASE blog_comments');

  return;
}

########################################################################
sub create_comments_table {
########################################################################
  my ( $dbi, $options ) = @_;

  return
    if !$options->{create};

  $dbi->do('DROP TABLE IF EXISTS `blog_comments`');

  print {*STDERR} "creating new table blog_comments\n";

  my $sql = <<'END_OF_SQL';
CREATE TABLE `blog_comments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `comment` text,
  `guid` varchar(36) DEFAULT NULL,
  `url` varchar(32) DEFAULT NULL,
  `date_entered` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `status` enum('new','approved','declined') DEFAULT NULL,
  `ip` varchar(15) DEFAULT NULL,
  `parent_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
END_OF_SQL

  $dbi->do($sql);
  return;
}

########################################################################
sub create_session_table {
########################################################################
  my ( $dbi, $options ) = @_;

  return
    if !$options->{create};

  $dbi->do('DROP TABLE IF EXISTS session');

  print {*STDERR} "creating session table\n";

  my $sql = <<'END_OF_SQL';
CREATE TABLE session
 (
  id           int(11)      not null auto_increment primary key,
  session      varchar(50)  default null,
  login_cookie varchar(50)  not null default '',
  username     varchar(50)  not null default '',
  password     varchar(64)  default null,
  firstname    varchar(30)  default null,
  lastname     varchar(50)  default null,
  email        varchar(100) default null,
  prefs        text,
  updated      timestamp    not null default current_timestamp on update current_timestamp,
  added        datetime     default null,
  expires      datetime     default null
);
END_OF_SQL

  $dbi->do($sql);

  return;
}

########################################################################
sub create_admin_user {
########################################################################
  my ( $dbi, $options ) = @_;

  return
    if !$options->{create};

  print {*STDERR} "creating a new database user 'blog-admin'\n";

  $dbi->do(q{DROP USER 'blog-admin'@'%'});

  $dbi->do( sprintf q{CREATE USER 'blog-admin'@'%%' IDENTIFIED BY '%s'}, $options->{'db-pass'} );

  $dbi->do(q{GRANT ALL PRIVILEGES ON `blog_comments`.* TO 'blog-admin'@'%'});

  $dbi->do('FLUSH PRIVILEGES');

  return;
}

########################################################################
sub register_admin_user {
########################################################################
  my ( $dbi, $options ) = @_;

  return
    if !$options->{register} && !$options->{create};

  print {*STDERR} sprintf "registering user %s\n", $options->{username};

  $dbi->do( 'delete from session where username = ?', {}, $options->{username} );

  my $sql = <<'END_OF_SQL';
insert into session 
    (username, password, firstname, lastname, email, added)
  values 
    (?, sha2(?, 256), ?, ?, ?, now())
END_OF_SQL

  $dbi->do( $sql, {}, @{$options}{qw(username password firstname lastname email)} );

  return;
}

########################################################################
sub show_users {
########################################################################
  my ($dbi) = @_;

  my $sql = <<'END_OF_SQL';
select username, firstname, lastname, email, added 
  from session
  where username is not null and username <> ''
END_OF_SQL

  my $users = $dbi->selectall_arrayref( $sql, { Slice => {} } );

  print {*STDOUT} easy_table(
    data => $users,
    rows => [
      User         => 'username',
      'First Name' => 'firstname',
      'Last Name'  => 'lastname',
      'Email'      => 'email',
      'Date Added' => 'added',
    ],
    table_options => { headerText => 'Users' },
  );

  return;
}

########################################################################
sub main {
########################################################################
  my @option_specs = qw(
    help|h
    username|u=s
    password|p=s
    email|e=s
    firstname|f=s
    lastname|l=s
    db-user|U=s
    db-pass|P=s
    db-host|H=s
    create|c!
    register|r!
  );

  my %options = (
    register  => $TRUE,
    email     => $DEFAULT_ADMIN_EMAIL,
    register  => $TRUE,
    'db-user' => $DEFAULT_DB_USER,
    'db-host' => $DEFAULT_DB_HOST,
  );

  my $retval = GetOptions( \%options, @option_specs );

  if ( !$retval || $options{help} ) {
    usage();
  }

  croak "do not use 'blog-admin' to create the database!\n"
    if $options{'db-user'} eq 'blog-admin';

  croak "--password is required when --user is provided!\n"
    if $options{username} && !$options{password};

  if ( !$options{'db-pass'} ) {
    carp "--db-pass is require\n";
    usage();
  }

  # this is to prevent accidental dropping of the database when all
  # you meant to do was create an additional blog administrator
  if ( !defined $options{create} && $options{username} && $options{username} ne 'blog-admin' ) {
    croak "use ---nocreate, or --create if you are creating an initial admin user <> 'blog-admin'\n";
  }

  $options{create} //= $TRUE;

  $options{username} //= $DEFAULT_ADMIN_USER;

  # default user password will be same as database password
  $options{password} //= $options{'db-pass'};

  my $dbi = connect_db( \%options );

  create_database( $dbi, \%options );

  $dbi->do('USE blog_comments');

  create_comments_table( $dbi, \%options );

  create_session_table( $dbi, \%options );

  create_admin_user( $dbi, \%options );

  register_admin_user( $dbi, \%options );

  show_users($dbi);

  return 0;
}

exit main();

1;

__END__

=pod

=head1 SYNOPSIS

 create-datbase.pl Options

Example:

  ./create-database.pl -P foo --nocreate

=head1 OPTIONS

 --help|h
 --user|u=s        admin user name (default: blog-admin)
 --password|p=s    password for the new admin user (default: db-pass)
 --email|p=s       email address of admin user
 --firstname|f=s   name of admin user
 --lastname|l=s    last name of admin user
 --db-user|U=s     user to use to connect and create database (default: root)
 --db-pass|P=s     password for connect user
 --db-host|H=s     database host (default: 127.0.0.1)
 --create!|c       create resources (default: true, use --nocreate to only register a user)
 --register!|r     register a user (default: true)

=head2 Examples

=over 5

=item * Create a New Database with Default Admin

 create-database.pl --db-pass foo

=item * Register a New Admin User

 create-database.pl -u rlauer -p password --nocreate

=back

=head2 Notes

=over 5

=item 1. Only C<--db-pass> is a required option

=item 2. The C<--db-user> must have privileges to create databases and users

=item 3. Running this more than once without C<--nocreate> will drop
the database and remove the admin user

=item 4. The script will automatically create a non-root user
(C<blog-admin>) with privileges on all databases that has the same
password as the root user. B<This is not the same as the registered
user!>

=item 5. Don't use the C<blog-admin> user to create the database.

The point of this script is to create that user in your database. That
user will be dropped as part of the process if it already exists.

=back

=cut
