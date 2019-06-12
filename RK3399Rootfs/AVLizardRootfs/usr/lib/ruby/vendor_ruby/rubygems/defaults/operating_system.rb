unless ENV['DEBIAN_DISABLE_RUBYGEMS_INTEGRATION']

class << Gem
  OPERATING_SYSTEM_DEFAULTS = {
    :ssl_ca_cert => '/etc/ssl/certs/ca-certificates.crt'
  }

  alias :upstream_default_dir :default_dir
  def default_dir
    File.join('/', 'var', 'lib', 'gems', Gem::ConfigMap[:ruby_version])
  end

  alias :upstream_default_bindir :default_bindir
  def default_bindir
    File.join('/', 'usr', 'local', 'bin')
  end

  alias :upstream_default_path :default_path
  def default_path

    # FIXME remove (part of) this after we get rid of ruby 2.1 and 2.2
    extra_path = nil
    if RbConfig::CONFIG['ruby_version'] == '2.1.0'
      extra_path = File.join('/usr/share/rubygems-integration', '2.1')
    elsif RbConfig::CONFIG['ruby_version'] == '2.2.0'
      extra_path = File.join('/usr/share/rubygems-integration', '2.2')
    end

    arch = Gem::ConfigMap[:arch]
    api_version = Gem::ConfigMap[:ruby_version]

    upstream_default_path + [
      "/usr/lib/#{arch}/rubygems-integration/#{api_version}",
      File.join('/usr/share/rubygems-integration', api_version),
      extra_path,
      '/usr/share/rubygems-integration/all'
    ].compact
  end

end

if RUBY_VERSION >= '2.1' then
  class << Gem::BasicSpecification

    alias :upstream_default_specifications_dir :default_specifications_dir
    def default_specifications_dir
      File.join(Gem.upstream_default_dir, "specifications", "default")
    end

  end
end

end
