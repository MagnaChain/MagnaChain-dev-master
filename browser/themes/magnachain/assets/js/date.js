/*  1.生成所有年，默认激活当前年
	2.获取当前日期生成月、日，默认激活当前月、日
	3.判断n（函数可传两个参：年、月，点年生成月，点月生成日）是否存在    是：根据n生成日  否：说明第一次进，默认生成当前
	  当时2018是，要设置月是发行月
	注：年、月点击触发生成月、日，日点击触发数据查询。*/

$(function() {

		function date(year, month, day){

			//发行日期
			var beginDate = new Date(); //定义发行时间
			beginDate.setFullYear(2011);
		    beginDate.setMonth(1+1);//设置MGC发行日期月份
		    beginDate.setDate(3);
		    var beginYear = beginDate.getFullYear(); //发行年
		    var beginMonth = beginDate.getMonth(); //发行月
		    var beginDay = beginDate.getDate(); //发行日

		    //当前日期
		    var currentDate = new Date();
		    var currentYear = currentDate.getFullYear();
		    var currentMonth = currentDate.getMonth()+1;
		    var currentDay = currentDate.getDate();

		    if (year != null && month == null) { //用户点击了年，将用户点击时间设为点击年，1-当前月

		    	//用户点击的日期
			    var userDate = new Date();
			    userDate.setFullYear(year);
			    var userYear = userDate.getFullYear();
			    var userMonth = userDate.getMonth();
			    var userDay = userDate.getDate(); 

		    } else if (year != null && month != null){  //用户点击了月

		    	//用户点击的日期
			    var userDate = new Date();
			    userDate.setFullYear(year);
			    userDate.setMonth(month);
			    var userYear = userDate.getFullYear();
			    var userMonth = userDate.getMonth();
			    var userDay = userDate.getDate();

		    }

		    //计算本月有多少天
		    var allDay = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31][month-1];

		    //判断闰年
		    if(month == 2) {
			    if(year % 4 == 0 && year % 100 != 0 || year % 400 == 0) {
			     allDay = 29;
			    }
		    }
		 
		    //$(".year").empty();//每次清空

		 	if (year == null && month == null && day == null) {  //初始化，没有点击

		 		//年插入到year
			    for(var i = beginYear; i <= currentYear; i++) {

			    	if (i == currentYear) {
			    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
			    	} else{
			        	$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
			    	}

			    }
			    
			    //月插入到month
			    for(var i = 1; i <= currentMonth; i++) {

			    	if (i == currentMonth) {
			    		$(".month").append("<a href='javascript:void(0);' class='month-actived' id='month_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
			    	} else {
			    		$(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
			    	}
			        
			    }

			    //日插入到day
			    for(var i = 1; i <= currentDay; i++) {

			    	if (i == currentDay) {
			    		$(".day").append("<a href='javascript:void(0);' class='day-actived' id='day_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
			    	} else {
			    		$(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
			    	}
			        
			    }

			    $('.date').append(currentYear+'-'+currentMonth+'-'+currentDay);

			    registerClick();

		 	} else if (year != null && month == null && day == null) {	//点击年  年比开始小

		 		emptyDate();

		 		if (year <= beginYear) {

		 			//年插入到year
				    for(var i = beginYear; i <= currentYear; i++) {
				    	if (i == beginYear) {
				    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
				    	} else {
				    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    	}
				        
				    }

				    //月插入到month
				    for(var i = beginMonth; i <= 11+1; i++) {
				        $(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    }

		 		} else if (year > beginYear && year < currentYear) {  //点击年 年不是开始，不是当前

		 			//年插入到year
				    for(var i = beginYear; i <= currentYear; i++) {
				    	
				    	if (i == userYear) {
				    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
				    	} else {
				    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    	}
				        
				    }

				    //月插入到month
				    for(var i = 1; i <= 11+1; i++) {
				        $(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    }

		 		} else {  //点击年  年是当前

		 			//年插入到year
				    for(var i = beginYear; i <= currentYear; i++) {

				    	if (i == currentYear) {
				    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
				    	} else {
				    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    	}
				        
				    }

				    //月插入到month
				    for(var i = 1; i <= currentMonth; i++) {
				        $(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    }

		 		}

		 		registerClick();

			    //清空

		 	} else if (year != null && month != null && day == null) {  //点击月

		 		emptyDate();

		 		if (year == currentYear) {

		 			if (month == currentMonth ) {

		 				//年插入到year
					    for(var i = beginYear; i <= currentYear; i++) {

					    	if (i == currentYear) {
					    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //月插入到month
					    for(var i = 1; i <= currentMonth; i++) {

					    	if (i == currentMonth) {
					    		$(".month").append("<a href='javascript:void(0);' class='month-actived' id='month_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //日插入到day
					    for(var i = 1; i <= currentDay; i++) {
					        $(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    }

		 			} else {

		 				//年插入到year
					    for(var i = beginYear; i <= currentYear; i++) {

					    	if (i == currentYear) {
					    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //月插入到month
					    for(var i = 1; i <= currentMonth; i++) {

					    	if (i == userMonth) {
					    		$(".month").append("<a href='javascript:void(0);' class='month-actived' id='month_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //日插入到day
					    for(var i = 1; i <= allDay; i++) {
					        $(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    }

		 			}

		 		} else if (year == beginYear) {

		 			if (month == beginMonth) {

		 				//年插入到year
					    for(var i = beginYear; i <= currentYear; i++) {

					    	if (i == year) {
					    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //月插入到month
					    for(var i = beginMonth; i <= 12; i++) {

					    	if (i == month) {
					    		$(".month").append("<a href='javascript:void(0);' class='month-actived' id='month_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //日插入到day
					    for(var i = beginDay; i <= allDay; i++) {
					        $(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    }

		 			} else {

		 				//年插入到year
					    for(var i = beginYear; i <= currentYear; i++) {

					    	if (i == year) {
					    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //月插入到month
					    for(var i = beginMonth; i <= 12; i++) {

					    	if (i == month) {
					    		$(".month").append("<a href='javascript:void(0);' class='month-actived' id='month_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

					    //日插入到day
					    for(var i = 1; i <= allDay; i++) {
					        $(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    }

		 			}

		 		} else {

		 			//年插入到year
				    for(var i = beginYear; i <= currentYear; i++) {

				    	if (i == year) {
				    		$(".year").append("<a href='javascript:void(0);' class='year-actived' id='year_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
				    	} else {
				    		$(".year").append("<a href='javascript:void(0);' id='year_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    	}
				        
				    }

				    //月插入到month
				    for(var i = 1; i <= 12; i++) {

				    	if (i == month) {
				    		$(".month").append("<a href='javascript:void(0);' class='month-actived' id='month_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
				    	} else {
				    		$(".month").append("<a href='javascript:void(0);' id='month_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    	}
				        
				    }

				    //日插入到day
				    for(var i = 1; i <= allDay; i++) {
				        $(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    }

		 		}

		 		registerClick();
			    //清空

		 	} else {  //点击日

		 		$('.day').empty();
		 		$('.date').empty();
		 		$('.date').append(year+'-'+month+'-'+day);

		 		if (year == beginYear && month == beginMonth) {

		 			//日插入到day
				    for(var i = beginDay; i <= allDay; i++) {

				    	if (i == day) {
				    		$(".day").append("<a href='javascript:void(0);' class='day-actived' id='day_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
				    	} else {
				    		$(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    	}
				        
				    }

		 		} else if (year == currentYear && month >= currentMonth) {

		 			if (day > currentDay) {

		 				//日插入到day
					    for(var i = 1; i <= currentDay; i++) {

					    	if (i == currentDay) {
					    		$(".day").append("<a href='javascript:void(0);' class='day-actived' id='day_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

		 			} else {

		 				//日插入到day
					    for(var i = 1; i <= currentDay; i++) {

					    	if (i == day) {
					    		$(".day").append("<a href='javascript:void(0);' class='day-actived' id='day_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
					    	} else {
					    		$(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
					    	}
					        
					    }

		 			}

		 		} else {

		 			//日插入到day
				    for(var i = 1; i <= allDay; i++) {
				    	if (i == day) {
				    		$(".day").append("<a href='javascript:void(0);' class='day-actived' id='day_" + i + "'style='color: #333333; border-bottom:1px #333 solid; margin-left: 17px;'>"+i+"</a>");
				    	} else {
				    		$(".day").append("<a href='javascript:void(0);' id='day_" + i + "'style='color: #333333; margin-left: 17px;'>"+i+"</a>");
				    	}
				        
				    }

		 		}

			    registerClick();

		 	}

		}

		function emptyDate() {

			$('.year').empty();
			$('.month').empty();
			$('.day').empty();

		}

		var web_year = $('#web_date').attr('year');
		var web_month = $('#web_date').attr('month');
		var web_day = $('#web_date').attr('day');

		date();

		if (web_year != null) {

			date(web_year, web_month, web_day);

		}

    	registerClick();

    	function registerClick() {

    		$("a[id^='year_'").unbind("click");
    		//所有年的点击事件
    		$("a[id^='year_'").bind("click", function(event){

   				event.stopImmediatePropagation();

   				var year = event.target.id;
   				var year = year.substring(5);
   				
   				date(year, null, null);

   				/*$.post("/changeAdmin", {id:id}, function (data, status) {

        	       location.reload();

   				});*/
   				
   			});

    		$("a[id^='month_'").unbind("click");
    		//所有月的点击事件
			$("a[id^='month_'").bind("click", function(event){

   				event.stopImmediatePropagation();

   				var month = event.target.id;
   				var month = month.substring(6);

   				var a_year = $('.year-actived');
   				var year = $('.year').find(a_year).text();

   				date(year, month, null);

   				/*$.post("/changeAdmin", {id:id}, function (data, status) {

        	       location.reload();

   				});*/
   				
   			});

			$("a[id^='day_'").unbind("click");
			//所有日的点击事件
    		$("a[id^='day_'").bind("click", function(event){

    			$('#no-block').hide();
    			$('#div-loader').show();

   				event.stopImmediatePropagation();

   				var day = event.target.id;
   				var day = day.substring(4);
   				
   				var a_year = $('.year-actived');
   				var year = $('.year').find(a_year).text();

   				var a_month = $('.month-actived');
   				var month = $('.month').find(a_month).text();

   				date(year, month, day);

   				if (day.length == 2) {
   					history.pushState("", "", '/all-blocks/' + year + '-' + month + '-' + day);
   					$("#web_date").attr("year", year);
   					$("#web_date").attr("month", month);
   					$("#web_date").attr("day", day);
   				} else {
   					history.pushState("", "", '/all-blocks/' + year + '-' + month + '-0' + day);
   					$("#web_date").attr("year", year);
   					$("#web_date").attr("month", month);
   					$("#web_date").attr("day", "0"+day);
   				}

   				$.post("/date_block", {year:year, month:month, day:day}, function (data, status) {
   					console.log(year+"--"+month+"--"+day);
        	       	if (data != 0) {

						$('.tbody-block').empty();
						$('#div-loader').hide();

						var data = JSON.parse(data);

						for (var i = 0; i < data['block'].length; i++) {

							$('.tbody-block').append(   "<tr>" +
													        "<td class='' style='line-height: 51px;'>"+
													        	"<a href='/block/"+data['block'][i]['height']+"'>"+
																	"<span class='ellipsis ng-binding'>"+ data['block'][i]['height']+"</span>"+
																"</a></td>"+
													        "<td class='' style='text-align: left; line-height: 51px;'>"+ data['block'][i]['time'] +"</td>"+
													        "<td class='' style='text-align: left; line-height: 51px;'>"+ data['block'][i]['num'] +"</td>"+
													        "<td class='' style='text-align: left; line-height: 51px;'>"+ data['block'][i]['bits'] +"</td>"+
													        "<td class='' style='text-align: left; line-height: 51px;'>"+
													        	"<a href='/block/" +data['block'][i]['blockhash']+ "' style='float: left;'>"+
																"<span class='ellipsis ng-binding' style='white-space:nowrap;'>"+ data['block'][i]['blockhash'] +"</span>"+
																"</a>"+
															"</td>"+
													    "</tr>" );
						}
						
						if (data['pagination']!=null) {console.log(data['pagination']);data['pagination'] = parseInt(data['pagination']);console.log(data['pagination']);}

						if (data['pagination']>1) {

							$("#web_page").attr('page', '');
							$("#web_page").attr('page', data['pagination']);

							$('#pagination').find('li').empty();
							$("#pagination p:first").text("");
							$("#pagination p:first").text('共'+data['pagination']+"页 跳到")

							if (data['pagination']>4) {
								
								for(var i = 1; i <= 5; i++){

									if (i==1) {
										$('#pagination').append("<li><a href='javascript:void(0);' id='pagination_1' page='1' style='background-color: #ebebeb; color: #777777'>1</a></li>"); 
										test("#pagination_1");
									}else{
										$('#pagination').append("<li><a href='javascript:void(0);' id='pagination_"+i+"' page='"+i+"' style='color: #777777'>"+i+"</a></li>"); 
										test("#pagination_"+i);
									}

								}
	
								$('#pagination').append("<li><a href='javascript:void(0);' id='pagination_next' page='2' style='color: #777777'>&raquo;</a></li>"); 
								test("#pagination_next");
								
							} else {

								for(var i = 1; i <= data['pagination']; i++){
									if (i==1) {
										$('#pagination').append("<li><a href='javascript:void(0);' id='pagination_1' page='1' class='avtive' style='background-color: #ebebeb; color: #777777'>"+i+"</a></li>");
										test("#pagination_1");
									} else {
										$('#pagination').append("<li><a href='javascript:void(0);' id='pagination_"+i+"' page="+i+" style='color: #777777'>"+i+"</a></li>"); 
										test("#pagination_"+i);
									}
								}

							}

							$('#pagination').show();

						} 

					} else {
						
						$("#pagination").hide();
						$('.tbody-block').empty();
						$('#show-more').remove();
						$('#no-block').show();
						$('#div-loader').hide();

					}

   				});

   				registerPageClick();
   				
   			});

    	}

	});