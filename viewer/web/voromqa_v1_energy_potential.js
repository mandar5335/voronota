
var Module = typeof Module !== 'undefined' ? Module : {};

if (!Module.expectedDataFileDownloads) {
  Module.expectedDataFileDownloads = 0;
  Module.finishedDataFileDownloads = 0;
}
Module.expectedDataFileDownloads++;
(function() {
 var loadPackage = function(metadata) {

    var PACKAGE_PATH;
    if (typeof window === 'object') {
      PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
    } else if (typeof location !== 'undefined') {
      // worker
      PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
    } else {
      throw 'using preloaded data can only be done on a web page or in a web worker';
    }
    var PACKAGE_NAME = './voromqa_v1_energy_potential.data';
    var REMOTE_PACKAGE_BASE = 'voromqa_v1_energy_potential.data';
    if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
      Module['locateFile'] = Module['locateFilePackage'];
      err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
    }
    var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
  
    var REMOTE_PACKAGE_SIZE = metadata.remote_package_size;
    var PACKAGE_UUID = metadata.package_uuid;
  
    function fetchRemotePackage(packageName, packageSize, callback, errback) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', packageName, true);
      xhr.responseType = 'arraybuffer';
      xhr.onprogress = function(event) {
        var url = packageName;
        var size = packageSize;
        if (event.total) size = event.total;
        if (event.loaded) {
          if (!xhr.addedTotal) {
            xhr.addedTotal = true;
            if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
            Module.dataFileDownloads[url] = {
              loaded: event.loaded,
              total: size
            };
          } else {
            Module.dataFileDownloads[url].loaded = event.loaded;
          }
          var total = 0;
          var loaded = 0;
          var num = 0;
          for (var download in Module.dataFileDownloads) {
          var data = Module.dataFileDownloads[download];
            total += data.total;
            loaded += data.loaded;
            num++;
          }
          total = Math.ceil(total * Module.expectedDataFileDownloads/num);
          if (Module['setStatus']) Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
        } else if (!Module.dataFileDownloads) {
          if (Module['setStatus']) Module['setStatus']('Downloading data...');
        }
      };
      xhr.onerror = function(event) {
        throw new Error("NetworkError for: " + packageName);
      }
      xhr.onload = function(event) {
        if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
          var packageData = xhr.response;
          callback(packageData);
        } else {
          throw new Error(xhr.statusText + " : " + xhr.responseURL);
        }
      };
      xhr.send(null);
    };

    function handleError(error) {
      console.error('package error:', error);
    };
  
      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);
    
  function runWithFS() {

    function assert(check, msg) {
      if (!check) throw msg + new Error().stack;
    }

    function DataRequest(start, end, audio) {
      this.start = start;
      this.end = end;
      this.audio = audio;
    }
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.name = name;
        this.requests[name] = this;
        Module['addRunDependency']('fp ' + this.name);
      },
      send: function() {},
      onload: function() {
        var byteArray = this.byteArray.subarray(this.start, this.end);
        this.finish(byteArray);
      },
      finish: function(byteArray) {
        var that = this;

        Module['FS_createDataFile'](this.name, null, byteArray, true, true, true); // canOwn this data in the filesystem, it is a slide into the heap that will never change
        Module['removeRunDependency']('fp ' + that.name);

        this.requests[this.name] = null;
      }
    };

  
    function processPackageData(arrayBuffer) {
      Module.finishedDataFileDownloads++;
      assert(arrayBuffer, 'Loading data file failed.');
      assert(arrayBuffer instanceof ArrayBuffer, 'bad input to processPackageData');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      
          var compressedData = {"data":null,"cachedOffset":796311,"cachedIndexes":[-1,-1],"cachedChunks":[null,null],"offsets":[0,755,1528,2295,3054,3831,4604,5344,6107,6865,7644,8416,9187,9973,10738,11508,12263,13033,13779,14530,15280,16054,16804,17562,18313,19065,19861,20608,21369,22134,22880,23634,24384,25153,25903,26666,27409,28141,28892,29667,30429,31200,31963,32722,33500,34280,35016,35787,36553,37301,38073,38861,39613,40386,41152,41914,42693,43466,44240,44997,45761,46522,47280,48042,48851,49608,50378,51151,51912,52684,53436,54213,54971,55763,56519,57292,58082,58836,59592,60347,61113,61871,62629,63376,64139,64889,65657,66411,67159,67933,68692,69449,70217,70987,71759,72497,73272,74028,74779,75533,76306,77061,77813,78568,79318,80075,80821,81562,82293,83044,83794,84542,85291,86037,86814,87581,88323,89090,89850,90615,91358,92118,92879,93635,94374,95109,95892,96626,97365,98115,98871,99630,100374,101123,101853,102606,103350,104076,104845,105614,106395,107169,107950,108734,109485,110252,111005,111783,112552,113344,114134,114900,115651,116417,117181,117927,118656,119419,120167,120921,121665,122412,123201,123937,124677,125432,126170,126908,127619,128340,129069,129782,130503,131234,131969,132773,133536,134311,135071,135839,136611,137397,138169,138945,139716,140488,141289,142045,142827,143576,144337,145102,145875,146653,147415,148204,148962,149745,150537,151294,152057,152816,153570,154319,155072,155828,156599,157368,158130,158916,159664,160435,161205,161955,162724,163490,164242,164993,165757,166512,167270,168033,168772,169558,170324,171081,171826,172568,173315,174077,174831,175559,176306,177098,177868,178651,179410,180205,180974,181758,182532,183316,184078,184840,185634,186377,187124,187884,188620,189339,190073,190796,191524,192271,192996,193733,194516,195289,196049,196835,197611,198400,199159,199933,200701,201463,202219,203007,203756,204509,205253,205992,206742,207498,208230,208981,209730,210480,211217,212024,212790,213547,214336,215104,215878,216662,217427,218204,218988,219783,220541,221291,222060,222820,223577,224331,225090,225863,226614,227360,228143,228889,229660,230427,231178,231931,232696,233456,234223,234982,235734,236487,237253,238029,238788,239538,240269,241018,241761,242510,243260,244009,244775,245544,246297,247080,247845,248623,249403,250180,250957,251718,252492,253283,254044,254803,255584,256365,257126,257902,258681,259452,260211,260988,261749,262485,263231,263980,264716,265451,266182,266929,267680,268426,269191,269940,270713,271486,272255,273031,273787,274567,275344,276100,276872,277656,278408,279180,279923,280674,281435,282196,282944,283703,284471,285246,286004,286766,287517,288267,289005,289766,290523,291288,292011,292756,293556,294314,295085,295843,296611,297384,298142,298931,299692,300453,301244,302029,302810,303581,304346,305101,305888,306662,307422,308197,309006,309763,310513,311263,312020,312783,313527,314300,315039,315781,316570,317338,318117,318876,319643,320420,321176,321941,322712,323482,324269,325021,325779,326527,327285,328050,328795,329553,330311,331059,331838,332591,333359,334105,334853,335613,336371,337121,337862,338599,339362,340112,340836,341571,342320,343060,343808,344557,345301,346057,346826,347593,348325,349079,349832,350592,351356,352116,352841,353580,354371,355147,355931,356696,357464,358234,359009,359790,360551,361361,362105,362844,363574,364320,365042,365771,366506,367239,367983,368736,369498,370250,371027,371792,372574,373347,374118,374878,375664,376437,377168,377886,378630,379352,380073,380791,381516,382258,383016,383757,384526,385289,386068,386843,387617,388386,389149,389926,390676,391436,392187,392936,393691,394438,395196,395936,396697,397465,398213,398960,399716,400480,401236,401998,402732,403493,404259,405002,405733,406482,407234,407971,408711,409463,410218,410984,411748,412502,413259,414025,414777,415525,416267,417018,417813,418576,419342,420108,420855,421629,422411,423170,423944,424738,425489,426273,427043,427845,428609,429391,430151,430943,431700,432455,433178,433923,434651,435381,436130,436865,437584,438386,439151,439910,440695,441442,442215,442964,443752,444544,445285,446029,446783,447540,448307,449064,449798,450540,451339,452125,452881,453647,454425,455209,455967,456735,457527,458301,459091,459863,460627,461412,462183,462954,463743,464506,465257,466036,466815,467590,468378,469127,469903,470685,471421,472185,472944,473695,474452,475193,475946,476720,477476,478224,478982,479720,480490,481245,481997,482786,483516,484266,484999,485733,486471,487207,487927,488662,489440,490166,490894,491627,492385,493134,493840,494579,495347,496094,496851,497605,498340,499113,499855,500589,501393,502166,502925,503705,504495,505256,506008,506768,507558,508296,509052,509793,510535,511295,512001,512729,513503,514246,514973,515706,516446,517208,517912,518627,519416,520187,520953,521742,522514,523282,524058,524849,525611,526367,527138,527914,528671,529425,530184,530959,531711,532473,533224,533984,534760,535516,536279,537031,537781,538532,539297,540052,540780,541526,542302,543048,543786,544532,545276,546006,546755,547498,548232,548980,549726,550473,551227,551953,552677,553445,554200,554929,555677,556440,557172,557907,558690,559451,560223,561007,561791,562558,563323,564131,564905,565685,566460,567239,568007,568783,569576,570334,571110,571880,572642,573404,574175,574956,575713,576468,577237,577988,578732,579483,580261,581027,581764,582531,583265,584016,584792,585527,586258,586997,587732,588452,589187,589930,590666,591386,592133,592872,593597,594335,595092,595850,596594,597346,598098,598827,599591,600376,601143,601913,602689,603437,604208,605011,605802,606567,607343,608105,608887,609693,610466,611228,612012,612765,613533,614322,615059,615816,616568,617324,618086,618870,619627,620395,621151,621909,622651,623407,624176,624932,625693,626438,627177,627932,628694,629424,630172,630910,631655,632408,633197,633958,634730,635489,636230,636988,637771,638523,639297,640070,640858,641647,642385,643127,643867,644594,645338,646119,646902,647669,648443,649207,650018,650771,651531,652304,653060,653813,654604,655361,656130,656881,657631,658397,659127,659883,660631,661361,662100,662881,663634,664375,665102,665836,666610,667346,668115,668867,669621,670407,671168,671941,672712,673488,674286,675053,675832,676600,677374,678180,678901,679635,680377,681131,681910,682681,683433,684203,684968,685749,686501,687275,688016,688759,689523,690284,691046,691793,692537,693305,694041,694778,695490,696227,696996,697740,698444,699187,699923,700686,701451,702182,702921,703702,704442,705182,705939,706711,707485,708262,709046,709835,710601,711373,712132,712908,713717,714493,715269,716030,716810,717573,718320,719053,719842,720609,721349,722086,722872,723640,724353,725101,725876,726618,727357,728108,728895,729665,730420,731217,732004,732764,733535,734342,735122,735879,736666,737424,738165,738923,739709,740475,741228,741988,742762,743524,744297,745110,745856,746626,747410,748156,748913,749696,750468,751233,752009,752772,753525,754304,755039,755794,756565,757314,758071,758828,759602,760371,761179,761940,762703,763486,764219,765002,765747,766529,767307,768062,768837,769567,770300,771068,771803,772558,773283,774050,774769,775501,776252,776976,777748,778492,779270,779985,780760,781490,782230,782993,783758,784538,785269,786062,786864,787654,788401,789174,789944,790694,791454,792206,792980,793782,794567,795343,796100],"sizes":[755,773,767,759,777,773,740,763,758,779,772,771,786,765,770,755,770,746,751,750,774,750,758,751,752,796,747,761,765,746,754,750,769,750,763,743,732,751,775,762,771,763,759,778,780,736,771,766,748,772,788,752,773,766,762,779,773,774,757,764,761,758,762,809,757,770,773,761,772,752,777,758,792,756,773,790,754,756,755,766,758,758,747,763,750,768,754,748,774,759,757,768,770,772,738,775,756,751,754,773,755,752,755,750,757,746,741,731,751,750,748,749,746,777,767,742,767,760,765,743,760,761,756,739,735,783,734,739,750,756,759,744,749,730,753,744,726,769,769,781,774,781,784,751,767,753,778,769,792,790,766,751,766,764,746,729,763,748,754,744,747,789,736,740,755,738,738,711,721,729,713,721,731,735,804,763,775,760,768,772,786,772,776,771,772,801,756,782,749,761,765,773,778,762,789,758,783,792,757,763,759,754,749,753,756,771,769,762,786,748,771,770,750,769,766,752,751,764,755,758,763,739,786,766,757,745,742,747,762,754,728,747,792,770,783,759,795,769,784,774,784,762,762,794,743,747,760,736,719,734,723,728,747,725,737,783,773,760,786,776,789,759,774,768,762,756,788,749,753,744,739,750,756,732,751,749,750,737,807,766,757,789,768,774,784,765,777,784,795,758,750,769,760,757,754,759,773,751,746,783,746,771,767,751,753,765,760,767,759,752,753,766,776,759,750,731,749,743,749,750,749,766,769,753,783,765,778,780,777,777,761,774,791,761,759,781,781,761,776,779,771,759,777,761,736,746,749,736,735,731,747,751,746,765,749,773,773,769,776,756,780,777,756,772,784,752,772,743,751,761,761,748,759,768,775,758,762,751,750,738,761,757,765,723,745,800,758,771,758,768,773,758,789,761,761,791,785,781,771,765,755,787,774,760,775,809,757,750,750,757,763,744,773,739,742,789,768,779,759,767,777,756,765,771,770,787,752,758,748,758,765,745,758,758,748,779,753,768,746,748,760,758,750,741,737,763,750,724,735,749,740,748,749,744,756,769,767,732,754,753,760,764,760,725,739,791,776,784,765,768,770,775,781,761,810,744,739,730,746,722,729,735,733,744,753,762,752,777,765,782,773,771,760,786,773,731,718,744,722,721,718,725,742,758,741,769,763,779,775,774,769,763,777,750,760,751,749,755,747,758,740,761,768,748,747,756,764,756,762,734,761,766,743,731,749,752,737,740,752,755,766,764,754,757,766,752,748,742,751,795,763,766,766,747,774,782,759,774,794,751,784,770,802,764,782,760,792,757,755,723,745,728,730,749,735,719,802,765,759,785,747,773,749,788,792,741,744,754,757,767,757,734,742,799,786,756,766,778,784,758,768,792,774,790,772,764,785,771,771,789,763,751,779,779,775,788,749,776,782,736,764,759,751,757,741,753,774,756,748,758,738,770,755,752,789,730,750,733,734,738,736,720,735,778,726,728,733,758,749,706,739,768,747,757,754,735,773,742,734,804,773,759,780,790,761,752,760,790,738,756,741,742,760,706,728,774,743,727,733,740,762,704,715,789,771,766,789,772,768,776,791,762,756,771,776,757,754,759,775,752,762,751,760,776,756,763,752,750,751,765,755,728,746,776,746,738,746,744,730,749,743,734,748,746,747,754,726,724,768,755,729,748,763,732,735,783,761,772,784,784,767,765,808,774,780,775,779,768,776,793,758,776,770,762,762,771,781,757,755,769,751,744,751,778,766,737,767,734,751,776,735,731,739,735,720,735,743,736,720,747,739,725,738,757,758,744,752,752,729,764,785,767,770,776,748,771,803,791,765,776,762,782,806,773,762,784,753,768,789,737,757,752,756,762,784,757,768,756,758,742,756,769,756,761,745,739,755,762,730,748,738,745,753,789,761,772,759,741,758,783,752,774,773,788,789,738,742,740,727,744,781,783,767,774,764,811,753,760,773,756,753,791,757,769,751,750,766,730,756,748,730,739,781,753,741,727,734,774,736,769,752,754,786,761,773,771,776,798,767,779,768,774,806,721,734,742,754,779,771,752,770,765,781,752,774,741,743,764,761,762,747,744,768,736,737,712,737,769,744,704,743,736,763,765,731,739,781,740,740,757,772,774,777,784,789,766,772,759,776,809,776,776,761,780,763,747,733,789,767,740,737,786,768,713,748,775,742,739,751,787,770,755,797,787,760,771,807,780,757,787,758,741,758,786,766,753,760,774,762,773,813,746,770,784,746,757,783,772,765,776,763,753,779,735,755,771,749,757,757,774,769,808,761,763,783,733,783,745,782,778,755,775,730,733,768,735,755,725,767,719,732,751,724,772,744,778,715,775,730,740,763,765,780,731,793,802,790,747,773,770,750,760,752,774,802,785,776,757,211],"successes":[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]}
;
          compressedData.data = byteArray;
          assert(typeof LZ4 === 'object', 'LZ4 not present - was your app build with  -s LZ4=1  ?');
          LZ4.loadPackage({ 'metadata': metadata, 'compressedData': compressedData });
          Module['removeRunDependency']('datafile_./voromqa_v1_energy_potential.data');
    
    };
    Module['addRunDependency']('datafile_./voromqa_v1_energy_potential.data');
  
    if (!Module.preloadResults) Module.preloadResults = {};
  
      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }
    
  }
  if (Module['calledRun']) {
    runWithFS();
  } else {
    if (!Module['preRun']) Module['preRun'] = [];
    Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
  }

 }
 loadPackage({"files": [{"start": 0, "audio": 0, "end": 2146743, "filename": "/voromqa_v1_energy_potential"}], "remote_package_size": 800407, "package_uuid": "313fad71-96c5-41a6-9b07-a0f5df060365"});

})();
