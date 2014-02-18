function TDSnpFeatureSource(source) {
    this.source = source;
    this.uri = source.uri;
    this.refs = [];

    this.activityListeners = [];
    this.busy = 0;

    this.changeListeners = [];
}

TDSnpFeatureSource.prototype.setRef = function(ref) {
    this.refs = [ref];
    this.notifyChange();
}

TDSnpFeatureSource.prototype.addRef = function(ref) {
    this.refs.push(ref);
    this.notifyChange();
}

TDSnpFeatureSource.prototype.addChangeListener = function(listener) {
    this.changeListeners.push(listener);
}

TDSnpFeatureSource.prototype.notifyChange = function() {
    for (var li = 0; li < this.changeListeners.length; ++li) {
        try {
            this.changeListeners[li](this.busy);
        } catch (e) {
            console.log(e);
        }
    }
}

TDSnpFeatureSource.prototype.addActivityListener = function(listener) {
    this.activityListeners.push(listener);
}

TDSnpFeatureSource.prototype.notifyActivity = function() {
    for (var li = 0; li < this.activityListeners.length; ++li) {
        try {
            this.activityListeners[li](this.busy);
        } catch (e) {
            console.log(e);
        }
    }
}

TDSnpFeatureSource.prototype.getScales = function() {
    return [];
}

TDSnpFeatureSource.prototype.fetch = function(chr, min, max, scale, types, pool, callback) {
    var thisB = this;
    url = this.uri + '?chr=' + chr + '&min=' + min + '&max=' + max;
    if (this.refs) {
        for (var ri = 0; ri < this.refs.length; ++ri)
	       url += '&ref=' + this.refs[ri];
    }
    url += '&color=true';

    var req = new XMLHttpRequest();
    req.onreadystatechange = function() {
	if (req.readyState == 4) {
	    thisB.busy--;
            thisB.notifyActivity();
	    if (req.status >= 300) {
		callback('Error code ' + req.status, null);
	    } else {
		var jf = JSON.parse(req.response);
		var features = [];
		for (fi = 0; fi < jf.length; ++fi) {
		    var j = jf[fi];
		    
		    var f = new DASFeature();
		    f.segment = chr;
		    f.min = j['min'] | 0;
		    f.max = j['max'] | 0;
		    f.id = j.id;
            if (f.id == thisB.ref) 
                f.type = 'ref-snp'
            else
                f.type = 'snp';

		    f.score = j.score || j.score2;

            if (j.color)
                f.itemRgb = j.color;
		    
		    features.push(f);
		}
		callback(null, features);
	    }
	}
	
    };
    
    thisB.busy++;
    thisB.notifyActivity();

    req.open('GET', url, true);
    req.responseType = 'text';
    req.send('');
}

dalliance_registerSourceAdapterFactory('tdsnp', function(source) {
    return {features: new TDSnpFeatureSource(source)};
});

