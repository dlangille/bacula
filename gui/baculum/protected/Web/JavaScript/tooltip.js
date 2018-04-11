//Opentip.defaultStyle = 'rounded';
function showTip(el, title, description) {
	var tip = new Opentip(el, description, title, {
			stem: true, 
			fixed: true, 
			tipJoint: 'left middle',
			target: true,
			showOn: 'creation'
		});
}
