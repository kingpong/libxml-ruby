# $Id: libxml.rb 374 2008-07-11 04:51:41Z cfis $ 
# Please see the LICENSE file for copyright and distribution information 


# This is for backwards compatibility and is DEPRECATED
module LibXML
  module XML
    class Node
      def properties
        attributes
      end
    end
  
    class Attributes
      alias :each_attr :each
      alias :each_sibling :each
    end
  end
end