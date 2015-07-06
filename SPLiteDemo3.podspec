Pod::Spec.new do |s|


  s.name         = "SPLiteDemo3"
  s.version      = "1.0.0"
  s.summary      = "ARC and GCD Compatible Reachability Class for iOS and OS X."
  s.description  = <<-DESC
                  This application will help you to implement Spatialite functionalities in ios.
                   DESC
  s.homepage     = "https://github.com/neerajneeruz/SPLiteDemo3"
  s.license      = { :type => "MIT" }
 
  s.author              = { "Neeraj P K" => "neerajpk02@gmail.com" }
  s.social_media_url = 'https://www.facebook.com/neerajneeruz'
  s.platform     = :ios, "7.0"
  s.source       = { :git => "https://github.com/neerajneeruz/SPLiteDemo3.git",:tag => s.version.to_s }
  s.source_files = 'Spatialite2.3.1_LibEnv'
  s.requires_arc = false
end
