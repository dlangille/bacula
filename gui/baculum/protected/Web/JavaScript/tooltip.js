//Opentip.defaultStyle = 'rounded';
function showTip(el, title, description) {
	new Opentip(el, description, title, {
			stem: true, 
			fixed: true, 
			tipJoint: 'left middle',
			target: true,
			showOn: 'creation'
		});
}